// clang-format off
/**
 * 	AUTOGENERATED FILE!! DO NOT EDIT!!!!
 */


#include "ZcmLoop.hpp"
#include "AbstractData.hpp"
#include "DataDict.hpp"
#include "ZcmConversion.hpp"
#include <chrono>
#include <thread>
#include <common/messages/MsgChannels.hpp>
#include <common/messages/state_t.hpp>
#include <common/messages/imu_t.hpp>
#include <common/messages/lidar_t.hpp>
#include <common/messages/plane_fit_t.hpp>
#include <common/messages/global_update_t.hpp>
#include <common/utils/ZCMHandler.hpp>

using namespace std::chrono;

void ZcmLoop::run()
{
	ZCMHandler<state_t> STATE_handler;
	zcm.subscribe(maav::STATE_CHANNEL, &ZCMHandler<state_t>::recv, &STATE_handler);
	ZCMHandler<state_t> SIM_STATE_handler;
	zcm.subscribe(maav::SIM_STATE_CHANNEL, &ZCMHandler<state_t>::recv, &SIM_STATE_handler);
	ZCMHandler<lidar_t> HEIGHT_LIDAR_handler;
	zcm.subscribe(maav::HEIGHT_LIDAR_CHANNEL, &ZCMHandler<lidar_t>::recv, &HEIGHT_LIDAR_handler);
	ZCMHandler<imu_t> IMU_handler;
	zcm.subscribe(maav::IMU_CHANNEL, &ZCMHandler<imu_t>::recv, &IMU_handler);
	ZCMHandler<global_update_t> GLOBAL_UPDATE_handler;
	zcm.subscribe(maav::GLOBAL_UPDATE_CHANNEL, &ZCMHandler<global_update_t>::recv, &GLOBAL_UPDATE_handler);
	ZCMHandler<plane_fit_t> PLANE_FIT_handler;
	zcm.subscribe(maav::PLANE_FIT_CHANNEL, &ZCMHandler<plane_fit_t>::recv, &PLANE_FIT_handler);

	zcm.start();

	while (RUNNING)
	{
		while(STATE_handler.ready()) {
			double time = elapsedTime(static_cast<double>(STATE_handler.msg().utime) / 1e6);
			dict_->dict["STATE_attitude"]->addData(std::move(convertQuaternion(time, STATE_handler.msg().attitude)));
			dict_->dict["STATE_acceleration"]->addData(std::move(convertVector(time, STATE_handler.msg().acceleration)));
			dict_->dict["STATE_gyro_biases"]->addData(std::move(convertVector(time, STATE_handler.msg().gyro_biases)));
			dict_->dict["STATE_covariance"]->addData(std::move(convertMatrix(time, STATE_handler.msg().covariance)));
			dict_->dict["STATE_angular_velocity"]->addData(std::move(convertVector(time, STATE_handler.msg().angular_velocity)));
			dict_->dict["STATE_magnetic_field"]->addData(std::move(convertVector(time, STATE_handler.msg().magnetic_field)));
			dict_->dict["STATE_position"]->addData(std::move(convertVector(time, STATE_handler.msg().position)));
			dict_->dict["STATE_gravity"]->addData(std::move(convertVector(time, STATE_handler.msg().gravity)));
			dict_->dict["STATE_velocity"]->addData(std::move(convertVector(time, STATE_handler.msg().velocity)));
			dict_->dict["STATE_accel_biases"]->addData(std::move(convertVector(time, STATE_handler.msg().accel_biases)));
			STATE_handler.pop();
		}
		while(SIM_STATE_handler.ready()) {
			double time = elapsedTime(static_cast<double>(SIM_STATE_handler.msg().utime) / 1e6);
			dict_->dict["SIM_STATE_attitude"]->addData(std::move(convertQuaternion(time, SIM_STATE_handler.msg().attitude)));
			dict_->dict["SIM_STATE_acceleration"]->addData(std::move(convertVector(time, SIM_STATE_handler.msg().acceleration)));
			dict_->dict["SIM_STATE_gyro_biases"]->addData(std::move(convertVector(time, SIM_STATE_handler.msg().gyro_biases)));
			dict_->dict["SIM_STATE_covariance"]->addData(std::move(convertMatrix(time, SIM_STATE_handler.msg().covariance)));
			dict_->dict["SIM_STATE_angular_velocity"]->addData(std::move(convertVector(time, SIM_STATE_handler.msg().angular_velocity)));
			dict_->dict["SIM_STATE_magnetic_field"]->addData(std::move(convertVector(time, SIM_STATE_handler.msg().magnetic_field)));
			dict_->dict["SIM_STATE_position"]->addData(std::move(convertVector(time, SIM_STATE_handler.msg().position)));
			dict_->dict["SIM_STATE_gravity"]->addData(std::move(convertVector(time, SIM_STATE_handler.msg().gravity)));
			dict_->dict["SIM_STATE_velocity"]->addData(std::move(convertVector(time, SIM_STATE_handler.msg().velocity)));
			dict_->dict["SIM_STATE_accel_biases"]->addData(std::move(convertVector(time, SIM_STATE_handler.msg().accel_biases)));
			SIM_STATE_handler.pop();
		}
		while(HEIGHT_LIDAR_handler.ready()) {
			double time = elapsedTime(static_cast<double>(HEIGHT_LIDAR_handler.msg().utime) / 1e6);
			dict_->dict["HEIGHT_LIDAR_distance"]->addData(std::move(convertVector(time, HEIGHT_LIDAR_handler.msg().distance)));
			HEIGHT_LIDAR_handler.pop();
		}
		while(IMU_handler.ready()) {
			double time = elapsedTime(static_cast<double>(IMU_handler.msg().utime) / 1e6);
			dict_->dict["IMU_magnetometer"]->addData(std::move(convertVector(time, IMU_handler.msg().magnetometer)));
			dict_->dict["IMU_angular_rates"]->addData(std::move(convertVector(time, IMU_handler.msg().angular_rates)));
			dict_->dict["IMU_acceleration"]->addData(std::move(convertVector(time, IMU_handler.msg().acceleration)));
			IMU_handler.pop();
		}
		while(GLOBAL_UPDATE_handler.ready()) {
			double time = elapsedTime(static_cast<double>(GLOBAL_UPDATE_handler.msg().utime) / 1e6);
			dict_->dict["GLOBAL_UPDATE_position"]->addData(std::move(convertVector(time, GLOBAL_UPDATE_handler.msg().position)));
			dict_->dict["GLOBAL_UPDATE_attitude"]->addData(std::move(convertQuaternion(time, GLOBAL_UPDATE_handler.msg().attitude)));
			GLOBAL_UPDATE_handler.pop();
		}
		while(PLANE_FIT_handler.ready()) {
			double time = elapsedTime(static_cast<double>(PLANE_FIT_handler.msg().utime) / 1e6);
			dict_->dict["PLANE_FIT_z"]->addData(std::move(convertVector(time, PLANE_FIT_handler.msg().z)));
			dict_->dict["PLANE_FIT_roll"]->addData(std::move(convertVector(time, PLANE_FIT_handler.msg().roll)));
			dict_->dict["PLANE_FIT_z_dot"]->addData(std::move(convertVector(time, PLANE_FIT_handler.msg().z_dot)));
			dict_->dict["PLANE_FIT_pitch"]->addData(std::move(convertVector(time, PLANE_FIT_handler.msg().pitch)));
			PLANE_FIT_handler.pop();
		}
		std::this_thread::sleep_for(33ms);
	}

	zcm.stop();
}
