#define BOOST_TEST_MODULE StateTest
/**
 * Tests for both State and Kalman State
 */

#include <cmath>
#include <vector>

#include <Eigen/Eigen>
#include <boost/test/unit_test.hpp>
#include <sophus/so3.hpp>

#include <gnc/Constants.hpp>
#include <gnc/State.hpp>
#include "TestHelpers.hpp"

using namespace boost::unit_test;
using namespace Eigen;

using namespace maav::gnc;

BOOST_AUTO_TEST_CASE(ZeroStateTest)
{
    State base = State::zero(0);

    double quaternion_error = diff(base.attitude().unit_quaternion(), Quaterniond::Identity());
    BOOST_REQUIRE_CLOSE(quaternion_error, 0.0, 1e-5);

    BOOST_REQUIRE_EQUAL(base.angularVelocity(), Vector3d::Zero());
    BOOST_REQUIRE_EQUAL(base.position(), Vector3d::Zero());
    BOOST_REQUIRE_EQUAL(base.velocity(), Vector3d::Zero());
    BOOST_REQUIRE_EQUAL(base.acceleration(), Vector3d(0, 0, -constants::STANDARD_GRAVITY));
    BOOST_REQUIRE_EQUAL(base.timeSec(), 0.0);
    BOOST_REQUIRE_EQUAL(base.timeUSec(), 0);

    BOOST_REQUIRE_EQUAL(base.gyroBias(), Vector3d::Zero());
    BOOST_REQUIRE_EQUAL(base.accelBias(), Vector3d::Zero());
    BOOST_REQUIRE_EQUAL(base.gravity(), Vector3d(0, 0, -9.80665));
    BOOST_REQUIRE_EQUAL(base.magneticFieldVector(), constants::ANN_ARBOR_MAGNETIC_FIELD);
}

BOOST_AUTO_TEST_CASE(StateModificationTest)
{
    State base = State::zero(0);

    Vector3d vec(1, 2, 3);
    base.position() = vec;
    BOOST_REQUIRE_EQUAL(base.position(), vec);
}

BOOST_AUTO_TEST_CASE(UnweightedGaussianTest)
{
    std::array<State, State::N> states;
    states.fill(State::zero(200));
    std::array<double, State::N> weights;
    weights.fill(1.0 / static_cast<double>(State::N));

    // clang-format off
    // Points randomly sampled from a normal distribution in matlab
    // pts matrix in matlab script
    Eigen::Matrix<double, State::DoF, State::N> points;
    points << 
    0,                      0.918252798893386,	-1.13103735287684,	0.431702673972737,	0.159610374347848,	-0.654778477715828,	-0.217105808102122,	0.171557034854737,	1.79175519692014,	1.38669724863435,	-0.675907950129599,	1.51963015413033,	0.363220406178476,	-0.0315724886399453,	0.357882128216425,	-0.102629475247372,	-0.0621608739420303,	0.745913177198743,	0.705523985255030,	0.709608775820411,	0.336228344458471,	-0.604606198075897,	0.359131785153193,	0.816282430586742,	0.244796194623719,	0.518085782255152,	0.363961918490578,	-0.151937267749079,	0.147145701627314,	-0.394203907179873,	0.269217727301303,
    0,                      0.625683918641754,	-1.47596027582011,	0.217361723055846,	-0.671317234692967,	-0.269833817498929,	-0.132590803395396,	-0.0367427597874124,	2.24638575893937,	0.950765959529243,	-0.729561819055400,	1.50631557531225,	0.467154059286452,	0.0561898147040021,	0.125115915841110,	-0.114904636888106,	-0.111279997076490,	0.951446527700913,	1.04189453529354,	1.05067718730497,	0.103022837568348,	-0.604624297273630,	0.0271613487951894,	0.529542731154400,	0.251758287334975,	0.972498136314087,	0.158475829701016,	-0.0519244090755047,	0.0883917832175717,	-0.0910082948711781,	0.531488237944855,
    0,                      0.352873394945553,	-0.735775143574067,	0.387306216265502,	-0.0518175073461157,	-0.112265736219207,	-0.443193159293742,	-0.218183624970188,	1.06080873060629,	1.10971188815619,	-0.578218951364785,	1.05641172837302,	0.228207532354950,	0.258864916251524,	-0.131637781295134,	-0.416342678649320,	-0.421392680534600,	0.683592740772138,	0.577739045649803,	0.578438209480344,	0.399468142018039,	-0.276961647097123,	0.0456943828454021,	0.684891676787557,	-0.0543685811754206,	0.762113700445527,	0.287653546848091,	-0.0849062545125001,	0.101401320159796,	-0.325340511722177,	0.0513086497005776,
    1,                      1.36420957585139,	0.339921728402773,	1.76452372959877,	0.565356891825153,	0.918475881608329,	0.772440184119721,	0.484156341108377,	2.12018009144700,	1.35293057229726,	0.701057962269533,	1.74083142871431,	1.27699495662124,	1.00488127694047,	1.04964784884277,	0.674328877790621,	0.904520675654221,	1.76027126256113,	1.96533766445163,	1.56269076528484,	0.721022755800256,	0.502787678615063,	1.30141421274945,	1.20435505632097,	1.27383643216623,	1.67666264869081,	1.46174399189789,	0.536015740091020,	1.02479941573280,	0.600905626415929,	0.974011464002712,
    4,                      4.68814962065909,	3.46432265344117,	3.97662977488098,	3.92275251067749,	3.59919489654012,	4.04275164422838,	4.05913902422539,	5.35529611937421,	4.54934559963805,	3.43780727996192,	4.86998008224712,	3.94020627131087,	4.14298345907791,	4.20253629781085,	3.91327638726323,	3.62992853674074,	4.72636502262194,	4.52063590260429,	4.46543053630316,	4.29584133605698,	3.60043824796245,	3.78728437736686,	4.50220116130922,	3.93842349475599,	4.25193047083571,	3.94757000653344,	3.88021433510995,	3.71138029028930,	3.99638561727537,	4.78880689759674,
    -1.20000000000000,	   -0.765758770471349,	-1.98256460570198,	-0.512562384622510,	-1.27666904010707,	-1.35766236687880,	-1.74032244245863,	-1.43573433411424,	-0.0188965062305408,	-0.581405017119850,	-1.69733097846659,	-0.392086795694585,	-1.11851552711224,	-0.574895174695301,	-0.840724584149414,	-1.57790978329489,	-1.68454126131150,	-0.530645690110715,	-0.242456751296629,	-0.391620985997300,	-1.57537477956169,	-2.21428689122985,	-1.16891082930636,	-0.689679100720868,	-0.892256194530884,	-0.287839181370945,	-0.677104185218325,	-1.61121951701287,	-1.21390690393432,	-1.53307086879408,	-1.18798076921342,
    0.100000000000000,	    0.794649087257169,	-1.05192309478275,	0.505432482603863,	0.351352661238695,	-0.0665454821773397,	-0.525244260113371,	0.499898284177741,	1.55756637990893,	1.09063167039261,	-0.443967725409231,	1.12662433552438,	0.304089350068563,	0.356163725874290,	0.508193081263917,	0.151533476845151,	0.319759945754561,	0.700861182707631,	0.718095955956812,	0.942526031078181,	0.0992059363879434,	-0.899480894027940,	0.576833698883087,	0.779295475887019,	0.459104388546590,	0.854572078831757,	0.590223135174000,	-0.295559366865772,	0.240903188376310,	-0.264447383749271,	-0.215461774241221,
    0.200000000000000,	    1.30661310067351,	-1.19154303351285,	0.775071191313210,	0.162463921479047,	-0.238563347149103,	-0.515711568799856,	0.267673327456117,	2.11121188519089,	1.17490661768686,	-0.606203135956056,	1.71197140062505,	0.775979450292940,	0.359834266339101,	0.187972879945230,	-0.0105644263340364,	0.326205383782603,	1.03193966315507,	0.987057268678853,	1.19217347711025,	0.214310121818584,	-1.10704007369545,	0.555230405809283,	0.705275071509030,	0.481550036529767,	1.01132111033416,	0.841975426740156,	-1.04355710558122,	0.255782083718924,	-0.0246608890919580,	-0.313711550112503,
    -0.0500000000000000,	0.495309412228021,	-0.699349573507454,	0.407744926310235,	-0.149899247495813,	-0.398498109389915,	-0.411357401538229,	-0.0178644134900349,	1.07971614211282,	0.834902440308436,	-0.653631525787266,	0.885986672792930,	0.191398606872145,	0.133933994920641,	-0.155356454262807,	-0.406839938064647,	0.0601713763770141,	0.309421183347441,	0.996901211157139,	0.558817319273912,	0.362125417935074,	-0.634387664876805,	0.0384191979931612,	0.465899035838076,	0.0638977606489585,	0.354922365263861,	0.341504460851750,	-0.418137334583595,	0.0178596008956761,	-0.308625555150315,	-0.168198480452012,
    0,	                    0.564965164102136,	-0.766725028780445,	0.636852440309742,	-0.413162733400745,	-0.309799089224653,	-0.358557861480546,	-0.278175075954745,	1.38229818194778,	0.579817012231353,	-0.176432381109121,	1.06441138452697,	0.458309466407307,	0.0841547146971428,	0.317424553054932,	-0.185108544218010,	-0.136944697398101,	0.690364172031964,	1.11902436025642,	0.399840533101670,	-0.277375082495022,	-0.743849366329114,	0.569995630358849,	0.419144963222987,	0.369209618983668,	0.744292096777393,	0.0647338148708292,	-0.161462318077122,	-0.178090471755958,	-0.787979060892831,	0.110068764934207,
    0,	                    0.201070801965751,	-0.940973209649685,	1.00576034970647,	-0.121468484041856,	-0.0174012258598190,	-0.457398994146389,	-0.167656708954603,	1.38964042188781,	0.745340253080439,	-0.145812346114171,	0.781894512192929,	0.259199189395018,	0.234069101173456,	0.123476646968311,	-0.210070551849308,	0.0779528941777688,	0.669355020530045,	1.23174201171160,	0.877446522529691,	-0.519041060378352,	-0.821307807807359,	0.731305665362998,	0.419765685319387,	0.584848800942173,	1.01595159214049,	0.513267938367007,	-0.240286804213820,	-0.0499783081364965,	-0.554814078539799,	-0.337361457194502,
    0,	                    0.676390039639120,	-0.595801464525076,	0.191333411731536,	-0.162063914664728,	-0.494397179339072,	-0.0836253119123113,	0.173748159599928,	0.762665815449558,	0.553648645132656,	-0.753466112846059,	0.861004150820026,	0.219406806590729,	0.320279484055910,	0.303710655232964,	0.0441407380266622,	-0.0288115051386436,	0.602209646080604,	1.16149428742060,	0.402410510714381,	0.281561890520244,	-0.874213374612020,	0.0712389976080121,	0.183821894922730,	0.471461869920315,	0.244238494265965,	0.289584940917366,	-0.482958197138981,	-0.108077873349003,	-0.695995624348517,	-0.0609983964451544,
    0,	                    0.898597128751862,	-0.637004910242506,	0.758399610276222,	-0.0488764832385476,	-0.615668291912702,	-0.613210069837356,	-0.281360807861403,	1.89889843258298,	0.981497824141290,	-0.476526681512326,	1.44158370812008,	0.463225886486482,	-0.0176596706093359,	0.0489848179319912,	-0.586924979793850,	0.130634888655117,	0.487283287826973,	1.06458054418252,	0.770340254502814,	0.343260767056957,	-0.835112724852062,	0.640518255561867,	0.750679770941687,	0.0791004011584109,	0.908718763593750,	0.340749007338046,	-0.399768193673545,	-0.372743811329220,	-0.485529333609657,	-0.167932529337365,
    0,	                    0.903054561152776,	-1.04194314905870,	0.316072498045338,	-0.364391668325109,	-0.443066447221381,	-0.566610098359518,	0.292309534188615,	2.01881084707618,	0.355869165488887,	-0.246115123284276,	1.09524331283737,	0.164229047846847,	0.379866467080150,	0.248303390506744,	0.0852262468817724,	-0.0100480017414301,	0.792910810104799,	1.45611166934168,	0.758323985661011,	-0.474175490814201,	-1.55584461003239,	0.393435068048496,	0.251072962561847,	0.496556002473556,	0.769055614914542,	-0.210971656937698,	-0.755996190519515,	-0.781528301449326,	-0.529874206480733,	0.195703640022430,
    0,	                    0.769207602267706,	-1.00663509281540,	0.528379587433493,	0.0446744470489348,	-0.353454666762592,	-0.625789916736223,	0.0252533214647981,	1.64625536910004,	0.877196112770083,	-0.645552398284740,	1.18125384982257,	-0.0103024022386699,	0.219214162158720,	0.113342162959981,	-0.200946478903598,	-0.0566089954804166,	0.969940176607157,	1.08693975099410,	0.998579278622828,	0.0888378350367268,	-1.08630345580628,	0.296741666565936,	0.362867219967167,	0.372206266946727,	1.00362262291438,	0.538587499199815,	-0.814920152008509,	-0.326954977547066,	-0.530743084794498,	-0.246993335476099;
    // clang-format on

    // Correct mean of points as computed by matlab
    // mu vector in matlab script
    Sophus::SO3d mean_rot(Eigen::Quaterniond(0.503, -0.002, -0.820, 0.273));
    mean_rot.normalize();
    Eigen::Vector3d mean_position(1.11613912057654, 4.13571638240964, -1.06367523292670);
    Eigen::Vector3d mean_velocity(0.318222115205558, 0.373708482579212, 0.100864046017045);
    Eigen::Vector3d mean_gyro_bias(0.154878876151579, 0.202532786147264, 0.112062628526766);
    Eigen::Vector3d mean_accel_bias(0.208668866493522, 0.128760963871250, 0.168319160484744);

    // Put points in states
    states[0].attitude() = mean_rot;
    states[0].position() = {1, 4, -1.2};
    states[0].velocity() = {0.1, 0.2, -0.05};
    states[0].gyroBias() = Eigen::Vector3d::Zero();
    states[0].accelBias() = Eigen::Vector3d::Zero();
    for (size_t i = 1; i < State::N; i++)
    {
        const State::ErrorStateVector& vec = points.block<State::DoF, 1>(0, i);
        states[i].attitude() = mean_rot * Sophus::SO3d::exp(vec.segment<3>(0));
        states[i].position() = vec.segment<3>(3);
        states[i].velocity() = vec.segment<3>(6);
        states[i].gyroBias() = vec.segment<3>(9);
        states[i].accelBias() = vec.segment<3>(12);
    }

    // Computed covariance of generated points
    // C matrix in matlab script
    // clang-format off
    State::CovarianceMatrix output;
    output <<     
    0.4703,    0.4723,    0.3175,    0.2360,    0.2427,    0.2890,    0.3347,    0.4440,    0.2753,    0.2966,    0.2889,    0.2561,    0.3880,    0.3723,    0.3813,
    0.4723,    0.5568,    0.3371,    0.2831,    0.2760,    0.3285,    0.3443,    0.4729,    0.2916,    0.3354,    0.3306,    0.2675,    0.4064,    0.4358,    0.4141,
    0.3175,    0.3371,    0.2582,    0.1857,    0.1713,    0.2311,    0.2341,    0.3166,    0.2090,    0.2156,    0.2222,    0.1752,    0.2875,    0.2476,    0.2830,
    0.2360,    0.2831,    0.1857,    0.2215,    0.1399,    0.2380,    0.2196,    0.3158,    0.1900,    0.2398,    0.2593,    0.1791,    0.2771,    0.2864,    0.2846,
    0.2427,    0.2760,    0.1713,    0.1399,    0.1978,    0.1808,    0.1820,    0.2526,    0.1631,    0.1770,    0.1477,    0.1593,    0.2278,    0.2516,    0.2314,
    0.2890,    0.3285,    0.2311,    0.2380,    0.1808,    0.3093,    0.2847,    0.3794,    0.2281,    0.2695,    0.2992,    0.2200,    0.3204,    0.3490,    0.3479,
    0.3347,    0.3443,    0.2341,    0.2196,    0.1820,    0.2847,    0.3357,    0.4288,    0.2456,    0.2674,    0.2984,    0.2334,    0.3442,    0.3623,    0.3685,
    0.4440,    0.4729,    0.3166,    0.3158,    0.2526,    0.3794,    0.4288,    0.6008,    0.3400,    0.3601,    0.3909,    0.3169,    0.4774,    0.4890,    0.4998,
    0.2753,    0.2916,    0.2090,    0.1900,    0.1631,    0.2281,    0.2456,    0.3400,    0.2245,    0.2200,    0.2321,    0.2015,    0.2998,    0.2810,    0.2961,
    0.2966,    0.3354,    0.2156,    0.2398,    0.1770,    0.2695,    0.2674,    0.3601,    0.2200,    0.2977,    0.2961,    0.2209,    0.3348,    0.3619,    0.3285,
    0.2889,    0.3306,    0.2222,    0.2593,    0.1477,    0.2992,    0.2984,    0.3909,    0.2321,    0.2961,    0.3463,    0.2169,    0.3416,    0.3664,    0.3581,
    0.2561,    0.2675,    0.1752,    0.1791,    0.1593,    0.2200,    0.2334,    0.3169,    0.2015,    0.2209,    0.2169,    0.2279,    0.2640,    0.2989,    0.2871,
    0.3880,    0.4064,    0.2875,    0.2771,    0.2278,    0.3204,    0.3442,    0.4774,    0.2998,    0.3348,    0.3416,    0.2640,    0.4514,    0.4171,    0.4198,
    0.3723,    0.4358,    0.2476,    0.2864,    0.2516,    0.3490,    0.3623,    0.4890,    0.2810,    0.3619,    0.3664,    0.2989,    0.4171,    0.5321,    0.4463,
    0.3813,    0.4141,    0.2830,    0.2846,    0.2314,    0.3479,    0.3685,    0.4998,    0.2961,    0.3285,    0.3581,    0.2871,    0.4198,    0.4463,    0.4515;
    // clang-format on

    constexpr double tol = 1e-3;

    // Check c++ implementation of mean + cov to values computed by matlab
    State computed_gaussian = State::compute_gaussian(states, weights, weights);
    BOOST_CHECK_LE(
        diff(computed_gaussian.attitude().unit_quaternion(), mean_rot.unit_quaternion()), tol);
    BOOST_CHECK_LE((computed_gaussian.position() - mean_position).norm(), tol);
    BOOST_CHECK_LE((computed_gaussian.velocity() - mean_velocity).norm(), tol);
    BOOST_CHECK_LE((computed_gaussian.gyroBias() - mean_gyro_bias).norm(), tol);
    BOOST_CHECK_LE((computed_gaussian.accelBias() - mean_accel_bias).norm(), tol);
    BOOST_CHECK_LE((computed_gaussian.covariance() - output).norm(), tol);
}
