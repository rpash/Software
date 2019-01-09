#define BOOST_TEST_MODULE UnscentedTransformTest
/**
 * Tests for both State and Kalman State
 */

#include <cmath>
#include <vector>

#include <yaml-cpp/yaml.h>
#include <Eigen/Eigen>
#include <boost/test/unit_test.hpp>
#include <sophus/so3.hpp>

#include <gnc/State.hpp>
#include <gnc/kalman/UnscentedTransform.hpp>
#include "TestHelpers.hpp"

using namespace boost::unit_test;
using namespace Eigen;

using namespace maav::gnc::kalman;
using namespace maav::gnc;

State identity(const State& state) { return state; }
BOOST_AUTO_TEST_CASE(IdentityTransformTest)
{
    UnscentedTransform<State> id(identity, 0.1, 2, 0);

    State state = State::zero(0);

    State transformed_state = id(state);

    constexpr double tol = 1e-5;
    BOOST_CHECK_LE(std::abs(diff(state.attitude(), transformed_state.attitude())), tol);
    BOOST_CHECK_LE(std::abs(diff(state.position(), transformed_state.position())), tol);
    BOOST_CHECK_LE(std::abs(diff(state.velocity(), transformed_state.velocity())), tol);
}

BOOST_AUTO_TEST_CASE(YAMLRead)
{
    YAML::Node config = YAML::Load("alpha: 0.1\nbeta: 2.0\nkappa: 0.1");
    UnscentedTransform<State> id(config);
    id.set_transformation(identity);

    double w_m_0 = -98.337748344371000;
    double w_c_0 = -95.347748344371000;
    double w = 3.311258278145700;

    constexpr double tol = 1e-5;

    BOOST_CHECK_LE(std::abs(w_m_0 - id.m_weights()[0]), tol);
    BOOST_CHECK_LE(std::abs(w_c_0 - id.c_weights()[0]), tol);
    BOOST_CHECK_LE(std::abs(w - id.m_weights()[1]), tol);
    BOOST_CHECK_LE(std::abs(w - id.c_weights()[1]), tol);
}

void compare_states(const State& state, const State::ErrorStateVector& error_state)
{
    Sophus::SO3d e_attitude = Sophus::SO3d::exp(error_state.segment<3>(0));
    Eigen::Vector3d e_position = error_state.segment<3>(3);
    Eigen::Vector3d e_velocity = error_state.segment<3>(6);

    constexpr double tol = 1e-3;
    BOOST_CHECK_LE(diff(state.attitude(), e_attitude), tol);
    BOOST_CHECK_LE(diff(state.position(), e_position), tol);
    BOOST_CHECK_LE(diff(state.velocity(), e_velocity), tol);
}

BOOST_AUTO_TEST_CASE(SigmaPointsTest)
{
    using UT = UnscentedTransform<State>;

    UT id(identity, 0.25, 2, 0);

    State state = State::zero(0);

    State::CovarianceMatrix Sigma;
    // clang-format off
	Sigma << 
    0.00214877000000000,	0.00162408000000000,	0.00184648000000000,	0.00190725000000000,	0.00176487000000000,	0.00168313000000000,	0.00172485000000000,	0.00174179000000000,	0.00161380000000000,	0.00172100000000000,	0.00199413000000000,	0.00195815000000000,	0.00218185000000000,	0.00196010000000000,	0.00185920000000000,
    0.00162408000000000,	0.00230756000000000,	0.00206267000000000,	0.00177343000000000,	0.00156778000000000,	0.00185279000000000,	0.00206892000000000,	0.00152064000000000,	0.00186009000000000,	0.00165395000000000,	0.00181817000000000,	0.00216135000000000,	0.00208399000000000,	0.00185858000000000,	0.00166769000000000,
    0.00184648000000000,	0.00206267000000000,	0.00257836000000000,	0.00188381000000000,	0.00164753000000000,	0.00173195000000000,	0.00219777000000000,	0.00174657000000000,	0.00186974000000000,	0.00202278000000000,	0.00203332000000000,	0.00231498000000000,	0.00227537000000000,	0.00196517000000000,	0.00172721000000000,
    0.00190725000000000,	0.00177343000000000,	0.00188381000000000,	0.00236100000000000,	0.00180915000000000,	0.00166065000000000,	0.00184529000000000,	0.00180948000000000,	0.00151961000000000,	0.00213590000000000,	0.00183564000000000,	0.00215819000000000,	0.00229474000000000,	0.00196585000000000,	0.00187005000000000,
    0.00176487000000000,	0.00156778000000000,	0.00164753000000000,	0.00180915000000000,	0.00238811000000000,	0.00159868000000000,	0.00184980000000000,	0.00190261000000000,	0.00141617000000000,	0.00159806000000000,	0.00207885000000000,	0.00211522000000000,	0.00194314000000000,	0.00195110000000000,	0.00153718000000000,
    0.00168313000000000,	0.00185279000000000,	0.00173195000000000,	0.00166065000000000,	0.00159868000000000,	0.00241980000000000,	0.00157382000000000,	0.00158822000000000,	0.00201095000000000,	0.00160049000000000,	0.00203363000000000,	0.00207612000000000,	0.00198270000000000,	0.00160748000000000,	0.00162273000000000,
    0.00172485000000000,	0.00206892000000000,	0.00219777000000000,	0.00184529000000000,	0.00184980000000000,	0.00157382000000000,	0.00275403000000000,	0.00183650000000000,	0.00186146000000000,	0.00185973000000000,	0.00218501000000000,	0.00253410000000000,	0.00257692000000000,	0.00223882000000000,	0.00160984000000000,
    0.00174179000000000,	0.00152064000000000,	0.00174657000000000,	0.00180948000000000,	0.00190261000000000,	0.00158822000000000,	0.00183650000000000,	0.00230578000000000,	0.00161381000000000,	0.00208531000000000,	0.00198497000000000,	0.00190952000000000,	0.00213472000000000,	0.00198036000000000,	0.00162662000000000,
    0.00161380000000000,	0.00186009000000000,	0.00186974000000000,	0.00151961000000000,	0.00141617000000000,	0.00201095000000000,	0.00186146000000000,	0.00161381000000000,	0.00214820000000000,	0.00184646000000000,	0.00198592000000000,	0.00206626000000000,	0.00210341000000000,	0.00167379000000000,	0.00165109000000000,
    0.00172100000000000,	0.00165395000000000,	0.00202278000000000,	0.00213590000000000,	0.00159806000000000,	0.00160049000000000,	0.00185973000000000,	0.00208531000000000,	0.00184646000000000,	0.00287229000000000,	0.00195179000000000,	0.00189844000000000,	0.00251964000000000,	0.00193426000000000,	0.00203311000000000,
    0.00199413000000000,	0.00181817000000000,	0.00203332000000000,	0.00183564000000000,	0.00207885000000000,	0.00203363000000000,	0.00218501000000000,	0.00198497000000000,	0.00198592000000000,	0.00195179000000000,	0.00259367000000000,	0.00213923000000000,	0.00238684000000000,	0.00193629000000000,	0.00180752000000000,
    0.00195815000000000,	0.00216135000000000,	0.00231498000000000,	0.00215819000000000,	0.00211522000000000,	0.00207612000000000,	0.00253410000000000,	0.00190952000000000,	0.00206626000000000,	0.00189844000000000,	0.00213923000000000,	0.00318114000000000,	0.00256235000000000,	0.00236813000000000,	0.00179746000000000,
    0.00218185000000000,	0.00208399000000000,	0.00227537000000000,	0.00229474000000000,	0.00194314000000000,	0.00198270000000000,	0.00257692000000000,	0.00213472000000000,	0.00210341000000000,	0.00251964000000000,	0.00238684000000000,	0.00256235000000000,	0.00315732000000000,	0.00256649000000000,	0.00239580000000000,
    0.00196010000000000,	0.00185858000000000,	0.00196517000000000,	0.00196585000000000,	0.00195110000000000,	0.00160748000000000,	0.00223882000000000,	0.00198036000000000,	0.00167379000000000,	0.00193426000000000,	0.00193629000000000,	0.00236813000000000,	0.00256649000000000,	0.00250280000000000,	0.00197438000000000,
    0.00185920000000000,	0.00166769000000000,	0.00172721000000000,	0.00187005000000000,	0.00153718000000000,	0.00162273000000000,	0.00160984000000000,	0.00162662000000000,	0.00165109000000000,	0.00203311000000000,	0.00180752000000000,	0.00179746000000000,	0.00239580000000000,	0.00197438000000000,	0.00250342000000000;
    // clang-format on
    state.covariance() = Sigma;

    id(state);

    const UT::SigmaPoints& sig_pts = id.last_sigma_points();
    const UT::TransformedPoints& trans_pts = id.last_transformed_points();

    // clang-format off
	/**
	 * Values from matlab
	 */
	Eigen::Matrix<double, State::DoF, State::N> sigma_points;
	sigma_points << 
0,	0.0448828683909574,	-0.0448828683909574,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
0,	0.0339232997930844,	-0.0339232997930844,	0.0318205473106375,	-0.0318205473106375,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
0,	0.0385687248177027,	-0.0385687248177027,	0.0196531726581282,	-0.0196531726581282,	0.0233113442427958,	-0.0233113442427958,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
0,	0.0398380704955177,	-0.0398380704955177,	0.00977833012628625,	-0.00977833012628625,	0.00160407251167491,	-0.00160407251167491,	0.0229820984793125,	-0.0229820984793125,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
0,	0.0368640794208542,	-0.0368640794208542,	0.00688996735568392,	-0.00688996735568392,	-0.000542691908249579,	0.000542691908249579,	0.00700468985604933,	-0.00700468985604933,	0.0279832258469935,	-0.0279832258469935,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
0,	0.0351567186226874,	-0.0351567186226874,	0.0171071450816776,	-0.0171071450816776,	-0.00293661130463760,	0.00293661130463760,	-0.000273483926762900,	0.000273483926762900,	0.00304456522331074,	-0.00304456522331074,	0.0268690410329044,	-0.0268690410329044,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
0,	0.0360281535688523,	-0.0360281535688523,	0.0225457670004535,	-0.0225457670004535,	0.00977007504891300,	-0.00977007504891300,	0.00254699943304524,	-0.00254699943304524,	0.00851096966773490,	-0.00851096966773490,	-0.00645329471870831,	0.00645329471870831,	0.0236546079188711,	-0.0236546079188711,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
0,	0.0363819912483354,	-0.0363819912483354,	0.00601506950665676,	-0.00601506950665676,	0.00497556749408664,	-0.00497556749408664,	0.00784091416316150,	-0.00784091416316150,	0.0124660987197845,	-0.0124660987197845,	0.00319274420808873,	-0.00319274420808873,	0.00512589709761655,	-0.00512589709761655,	0.0228851511008515,	-0.0228851511008515,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
0,	0.0337085742119105,	-0.0337085742119105,	0.0188660584794919,	-0.0188660584794919,	0.00351788475575619,	-0.00351788475575619,	-0.00471548196894525,	0.00471548196894525,	-0.000358182173677814,	0.000358182173677814,	0.0144244049076216,	-0.0144244049076216,	0.00757083140315519,	-0.00757083140315519,	0.00490081023183688,	-0.00490081023183688,	0.0140571298002917,	-0.0140571298002917,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
0,	0.0359477359144244,	-0.0359477359144244,	0.0104056130606423,	-0.0104056130606423,	0.0131006861904596,	-0.0131006861904596,	0.0194740198600327,	-0.0194740198600327,	-0.00100035318258131,	0.00100035318258131,	0.00392599427012776,	-0.00392599427012776,	0.00296003072923478,	-0.00296003072923478,	0.0153559376051468,	-0.0153559376051468,	0.0152295838871624,	-0.0152295838871624,	0.0157628989009916,	-0.0157628989009916,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
0,	0.0416527940842714,	-0.0416527940842714,	0.00916182085787662,	-0.00916182085787662,	0.00513413120436422,	-0.00513413120436422,	-0.00157850140358613,	0.00157850140358613,	0.0130130988245811,	-0.0130130988245811,	0.00969316282731112,	-0.00969316282731112,	0.0104365152764070,	-0.0104365152764070,	0.00133508430855825,	-0.00133508430855825,	0.00275151736317340,	-0.00275151736317340,	0.00522037844399776,	-0.00522037844399776,	0.0132285975030461,	-0.0132285975030461,	0,	0,	0,	0,	0,	0,	0,	0,
0,	0.0409012545501627,	-0.0409012545501627,	0.0200738252156975,	-0.0200738252156975,	0.00850530812885431,	-0.00850530812885431,	0.00800381402123934,	-0.00800381402123934,	0.0102016537330508,	-0.0102016537330508,	0.00599607780087372,	-0.00599607780087372,	0.0125949061605318,	-0.0125949061605318,	-0.00588130049697137,	0.00588130049697137,	0.00271295674617185,	-0.00271295674617185,	-0.0106778074545799,	0.0106778074545799,	-0.0135511774551296,	0.0135511774551296,	0.0114799042157809,	-0.0114799042157809,	0,	0,	0,	0,	0,	0,
0,	0.0455738335879644,	-0.0455738335879644,	0.0128132870090214,	-0.0128132870090214,	0.00530269662976548,	-0.00530269662976548,	0.00878713032558113,	-0.00878713032558113,	-0.000189406047829034,	0.000189406047829034,	0.00208075170701252,	-0.00208075170701252,	0.0180044081032681,	-0.0180044081032681,	0.00324692708547750,	-0.00324692708547750,	0.00245146925883396,	-0.00245146925883396,	0.0127590733960086,	-0.0127590733960086,	-0.00564362533225040,	0.00564362533225040,	3.75880146798830e-05	-3.75880146798830e-05,	0.00858943496679870,	-0.00858943496679870,	0,	0,	0,	0,
0,	0.0409419855699379,	-0.0409419855699379,	0.0111101640061570,	-0.0111101640061570,	0.00192681846751362,	-0.00192681846751362,	0.00436013533665930,	-0.00436013533665930,	0.00764112219156048,	-0.00764112219156048,	-0.00516764647334001,	0.00516764647334001,	0.0103586713478910,	-0.0103586713478910,	0.00544371213282041,	-0.00544371213282041,	-0.00245898270951968,	0.00245898270951968,	0.00424813715216233,	-0.00424813715216233,	-0.0132294180559307,	0.0132294180559307,	-0.000124150219842777,	0.000124150219842777,	0.00401707474188365,	-0.00401707474188365,	0.00931016656141474,	-0.00931016656141474,	0,	0,
0,	0.0388344163928517,	-0.0388344163928517,	0.00773298532591739,	-0.00773298532591739,	-0.00130890092682811,	0.00130890092682811,	0.00576829595553700,	-0.00576829595553700,	-0.00303339690707644,	0.00303339690707644,	0.00114252685013010,	-0.00114252685013010,	-0.00139368100012375,	0.00139368100012375,	0.00297847780383249,	-0.00297847780383249,	0.00733759772453821,	-0.00733759772453821,	0.0110068494052818,	-0.0110068494052818,	-0.00126365937143548,	0.00126365937143548,	0.00401919660282255,	-0.00401919660282255,	0.0209746160029918,	-0.0209746160029918,	0.00543434182287664,	-0.00543434182287664,	0.00778009333453012,	-0.00778009333453012;
    // clang-format on

    constexpr double tol = 1e-5;
    for (size_t i = 0; i < sig_pts.size(); i++)
    {
        BOOST_REQUIRE_LE(diff(sig_pts[i].attitude(), trans_pts[i].attitude()), tol);
        BOOST_REQUIRE_LE(diff(sig_pts[i].position(), trans_pts[i].position()), tol);
        BOOST_REQUIRE_LE(diff(sig_pts[i].velocity(), trans_pts[i].velocity()), tol);
        compare_states(sig_pts[i], sigma_points.col(i));
    }
}
// TODO: Add more complex transformations
