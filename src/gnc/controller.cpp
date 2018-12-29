#include "gnc/controller.hpp"
#include <gnc/utils/ZcmConversion.hpp>
#include <iostream>

using std::cout;
using maav::mavlink::InnerLoopSetpoint;
using std::pair;
using namespace std::chrono;

namespace maav
{
namespace gnc
{
/*
 *      Helper to saturate values
 */
static double bounded(double value, const pair<double, double>& limits)
{
    assert(limits.first > limits.second);  // first is upper limit

    if (value > limits.first)
        return limits.first;
    else if (value < limits.second)
        return limits.second;
    else
        return value;
}

/*
 *      Helper to get heading out of state
 */
static double get_heading(const State& state)
{
    double q0 = state.attitude().unit_quaternion().w();
    double q1 = state.attitude().unit_quaternion().x();
    double q2 = state.attitude().unit_quaternion().y();
    double q3 = state.attitude().unit_quaternion().z();
    return atan2((q1 * q2) + (q0 * q3), 0.5 - (q2 * q2) - (q3 * q3));
}

Controller::Controller()
    : current_state(0),
      previous_state(0),
      current_target({Eigen::Vector3d(0, 0, 0), Eigen::Vector3d(0, 0, 0), 0.}),
      hold_altitude_setpoint(Waypoint{Eigen::Vector3d(0, 0, 0), Eigen::Vector3d(0, 0, 0), 0}),
      set_pause(false)
{
    set_control_state(ControlState::STANDBY);
    current_state.zero(0);
    previous_state.zero(0);
}

Controller::~Controller() {}
/*
 *      Sets current path and sets up class to move
 *      through path
 */
void Controller::set_path(const path_t& _path)
{
    converged_on_waypoint = false;
    set_pause = false;
    path = _path;
    path_counter = 0;
    current_target = ConvertWaypoint(path.waypoints[path_counter]);
}

void Controller::set_current_target(const Waypoint& new_target) { current_target = new_target; }
/*
 *      Control parameters set from message and vehicle
 *      parameters set from struct.  Vehicle parametes should
 *      not change once the vehicle is in flight but control
 *      params can be tuned.
 */
void Controller::set_control_params(const ctrl_params_t& ctrl_params)
{
    z_position_pid.setGains(ctrl_params.value[2].p, ctrl_params.value[2].i, ctrl_params.value[2].d);
    z_rate_pid.setGains(ctrl_params.rate[2].p, ctrl_params.rate[2].i, ctrl_params.rate[2].d);
    yaw_pid.setGains(ctrl_params.value[3].p, ctrl_params.value[3].i, ctrl_params.value[3].d);
    pitch_pid.setGains(
        ctrl_params.rate[0].p, ctrl_params.rate[0].i, ctrl_params.rate[0].d);  // x rate
    roll_pid.setGains(
        ctrl_params.rate[1].p, ctrl_params.rate[1].i, ctrl_params.rate[1].d);  // y rate
}

void Controller::set_control_params(const ctrl_params_t& ctrl_params, const Parameters& _veh_params)
{
    set_control_params(ctrl_params);
    // add something to veh params, see member varialbes
    veh_params = _veh_params;
}

/*
 *      Main run sequence
 *      Manages control behavior based on control state
 */
InnerLoopSetpoint Controller::run(const State& state)
{
    previous_state = current_state;
    current_state = state;
    dt = current_state.timeSec() - previous_state.timeSec();
    // Calculate discrete error derivatives?
    // Low pass filter?

    switch (current_control_state)
    {
        case ControlState::HOLD_ALT:
            return hold_altitude(hold_altitude_setpoint);

        case ControlState::STANDBY:
            return InnerLoopSetpoint();

        case ControlState::TEST_WAYPOINT:
            return move_to_current_target();

        case ControlState::TEST_PATH:
            if (set_pause == true && system_clock::now() < pause_timer)
            {
                return move_to_current_target();
            }
            else
            {
                set_pause = false;
            }

            if (converged_on_waypoint && path_counter < path.NUM_WAYPOINTS - 1)
            {
                ++path_counter;
                current_target = ConvertWaypoint(path.waypoints[path_counter]);
                converged_on_waypoint = false;
            }
            return move_to_current_target();

        default:
            assert(false);  // make sure all states get handled
    }

    assert(false);                                 // state logic should return
    return hold_altitude(hold_altitude_setpoint);  // dummy
}

/*
 *      Run method that controlls vehicle with xbox controller input
 *      to use run "maav-controller -x"
 */
InnerLoopSetpoint Controller::run(const XboxController& xbox_controller, const State& state)
{
    InnerLoopSetpoint setpoint;
    controller_run_loop_1 = controller_run_loop_2;
    controller_run_loop_2 = high_resolution_clock::now();
    auto dt = duration_cast<duration<double>>(controller_run_loop_2 - controller_run_loop_1);

    setpoint.thrust = (static_cast<float>(xbox_controller.left_joystick_y) / 32767 + 1) / 2;
    float roll = (static_cast<float>(xbox_controller.right_joystick_x) / 32767) * M_PI * 20 / 180;
    float pitch = (-static_cast<float>(xbox_controller.right_joystick_y) / 32767) * M_PI * 20 / 180;
    if (xbox_controller.left_trigger > 50)
    {
        yaw = yaw - dt.count() * veh_params.rate_limits[3].first;
        if (yaw < 0) yaw = yaw + 2 * M_PI;
    }
    if (xbox_controller.right_trigger > 50)
    {
        yaw = yaw + dt.count() * veh_params.rate_limits[3].first;
        if (yaw > 2 * M_PI) yaw = yaw - 2 * M_PI;
    }

    std::cout << yaw << '\n';

    // Todo: add pid controls, very hard to use as is (but it werks tho?!)

    Eigen::Quaterniond q_roll = {
        cos(roll / 2), cos(yaw) * sin(roll / 2), sin(yaw) * sin(roll / 2), 0};
    Eigen::Quaterniond q_pitch = {
        cos(pitch / 2), -sin(yaw) * sin(pitch / 2), cos(yaw) * sin(pitch / 2), 0};
    Eigen::Quaterniond q_yaw = {cos(yaw / 2), 0, 0, sin(yaw / 2)};

    setpoint.q = static_cast<Eigen::Quaternion<float>>(q_roll * q_pitch * q_yaw);
    return setpoint;
}

/*
 *      Get and set control state
 */
ControlState Controller::get_control_state() const { return current_control_state; }
bool Controller::set_control_state(const ControlState new_control_state)
{
    current_control_state = new_control_state;

    switch (current_control_state)
    {
        case ControlState::HOLD_ALT:
            cout << "Control mode switched to HOLD_ALT\n";
            break;

        case ControlState::STANDBY:
            cout << "Control mode switched to STANDBY\n";
            break;

        case ControlState::TEST_WAYPOINT:
            cout << "Control mode switched to TEST_WAYPOINT\n";
            break;

        case ControlState::TEST_PATH:
            cout << "Control mode switched to TEST_PATH\n";
            break;

        case ControlState::XBOX_CONTROLL:
            cout << "Control mode switched to XBOX_CONTROLL\n";
            break;

        default:
            assert(false);
    }

    return true;
}

mavlink::InnerLoopSetpoint Controller::move_to_current_target()
{
    InnerLoopSetpoint new_setpoint;

    Eigen::Vector2d xy_error_ned = {current_target.position.x() - current_state.position().x(),
        current_target.position.y() - current_state.position().y()};

    double heading = get_heading(current_state);
    Eigen::Vector2d xy_error_body = {
        xy_error_ned.x() * cos(heading) + xy_error_ned.y() * sin(heading),
        xy_error_ned.x() * -sin(heading) + xy_error_ned.y() * cos(heading)};
    Eigen::Vector2d vel_body = {
        current_state.velocity().x() * cos(heading) + current_state.velocity().y() * sin(heading),
        current_state.velocity().x() * -sin(heading) + current_state.velocity().y() * cos(heading)};

    // Pause at waypoints to slow down test run
    if (!set_pause && xy_error_ned.norm() < convergence_tolerance)
    {
        pause_timer = system_clock::now() + seconds(5);
        set_pause = true;
        converged_on_waypoint = true;
    }

    double y_error = xy_error_body.y();
    double y_error_dot = vel_body.y();
    double roll = roll_pid.run(y_error, -y_error_dot);
    roll = bounded(roll, veh_params.angle_limits[0]);
    Eigen::Quaterniond q_roll = {
        cos(roll / 2), cos(heading) * sin(roll / 2), sin(heading) * sin(roll / 2), 0};

    double x_error = xy_error_body.x();
    double x_error_dot = vel_body.x();
    double pitch = pitch_pid.run(-x_error, x_error_dot);
    pitch = bounded(pitch, veh_params.angle_limits[1]);
    Eigen::Quaterniond q_pitch = {
        cos(pitch / 2), -sin(heading) * sin(pitch / 2), cos(heading) * sin(pitch / 2), 0};

    Eigen::Quaterniond q_yaw = {
        cos(current_target.yaw * M_PI / 180 / 2), 0, 0, sin(current_target.yaw * M_PI / 180 / 2)};

    new_setpoint.thrust = calculate_thrust(current_target.position.z());
    new_setpoint.q = static_cast<Eigen::Quaternion<float>>(q_roll * q_pitch * q_yaw);

    return new_setpoint;
}

/*
 *      Calculates inner loop setpoint thrust
 *      to maintain height setpoint
 */
double Controller::calculate_thrust(const double height_setpoint)
{
    double height_error = height_setpoint - current_state.position().z();
    double vel = z_position_pid.run(height_error, current_state.velocity().z());

    vel = bounded(vel, veh_params.rate_limits[2]);

    double vel_error = vel - current_state.velocity().z();
    double thrust =
        -z_rate_pid.run(vel_error, current_state.acceleration().z());  // TODO: derivative control?

    thrust = bounded(thrust, veh_params.thrust_limits);

    return static_cast<float>(thrust);
}

/*
 *      Maintain steady altitude
 */
InnerLoopSetpoint Controller::hold_altitude(const Waypoint& hold_alt_wpt)
{
    current_target = hold_alt_wpt;
    return move_to_current_target();
}

}  // namespace gnc
}  // namespace maav
