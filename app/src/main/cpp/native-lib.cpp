#include <jni.h>
#include <string>
#include <android/sensor.h>

#include <cassert>

typedef float** window_t;
typedef float* xyz_t;

const u_int USEC = 1000000;
const u_int SENSOR_WINDOW_SIZE = 100;
const u_int XYZ_SIZE = 4;
const u_int SENSOR_FREQ = 20;
const u_int LOOPER_ID_USER = 3;
constexpr u_int SENSOR_REFRESH_RATE = u_int(USEC / SENSOR_FREQ);

class SensorDriver {
 private:
    int sampleIndex = 0;
    ASensorEventQueue *accelerometerEventQueue;
    ASensorManager *sensorManager;
    const ASensor *sensor;
    ALooper *looper;
    u_int sensorCode = -1;
 public:
    window_t window;

    SensorDriver(u_int sensor)
    {
        sensorCode = sensor;
        window = (window_t)malloc(SENSOR_WINDOW_SIZE * sizeof(float));

        for (int i = 0; i < SENSOR_WINDOW_SIZE; ++i)
        {
            window[i] = (xyz_t)malloc(XYZ_SIZE * sizeof(float));
        }
    }

    ~SensorDriver()
    {
        for (int i = 0; i < SENSOR_WINDOW_SIZE; ++i)
        {
            free(window[i]);
        }

        free(window);
    }

    void init()
    {
        sensorManager = ASensorManager_getInstance();
        assert(sensorManager != NULL);
        sensor = ASensorManager_getDefaultSensor(sensorManager, sensorCode);
        assert(sensor != NULL);
        looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
        assert(looper != NULL);
        accelerometerEventQueue = ASensorManager_createEventQueue(sensorManager, looper,
                                                                  LOOPER_ID_USER, NULL, NULL);
        assert(accelerometerEventQueue != NULL);
        auto status = ASensorEventQueue_enableSensor(accelerometerEventQueue,
                                                     sensor);
        assert(status >= 0);
        status = ASensorEventQueue_setEventRate(accelerometerEventQueue,
                                                sensor,
                                                SENSOR_REFRESH_RATE);
        assert(status >= 0);
        (void)status;
    }

    void sample()
    {

        ALooper_pollAll(0, NULL, NULL, NULL);

        ASensorEvent event;
        bool done = false;

        while(!done)
        {
            while (ASensorEventQueue_getEvents(accelerometerEventQueue, &event, 1) > 0)
            {
                timespec t;
                window[sampleIndex][0] = event.acceleration.x;
                window[sampleIndex][1] = event.acceleration.y;
                window[sampleIndex][2] = event.acceleration.z;
                clock_gettime(CLOCK_REALTIME, &t);
                window[sampleIndex][3] = (float)t.tv_nsec;
                sampleIndex++;
                if (sampleIndex == SENSOR_WINDOW_SIZE)
                    done = true;
            }
        }
        sampleIndex = 0;
    }
};

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_example_sphota_accelerometer_1test_MainActivity_init(
        JNIEnv *env,
        jobject)
{
    SensorDriver driver(ASENSOR_TYPE_ACCELEROMETER);
    driver.init();
    driver.sample();

    jclass cls = env->FindClass("[F");

    jfloatArray iniVal = env->NewFloatArray(XYZ_SIZE);
    jobjectArray outer = env->NewObjectArray(SENSOR_WINDOW_SIZE, cls, iniVal);

    for (int i = 0; i < SENSOR_WINDOW_SIZE; i++)
    {
        jfloatArray inner = env->NewFloatArray(XYZ_SIZE);
        env->SetFloatArrayRegion(inner, 0, XYZ_SIZE, driver.window[i]);
        env->SetObjectArrayElement(outer, i, inner);
        env->DeleteLocalRef(inner);
    }
    return outer;
}
