#include <jni.h>
#include "TouchControlsInterface.h"
#include "JNITouchControlsUtils.h"
#include "SDL_beloko_extra.h"

#include "TouchControlsInterface.h"
#include "touch_interface.h"

#include "game_interface.h"

#include <unistd.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,"JNITouchControlsUtils", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "JNITouchControlsUtils", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR,"JNITouchControlsUtils", __VA_ARGS__))


extern "C"
{

static int android_screen_width;
static int android_screen_height;
int android_audio_rate;

JNIEnv* env_;

static jclass NativeLibClass = 0;
static jmethodID swapBuffersMethod = 0;
void swapBuffers()
{
	if (NativeLibClass == 0)
	{
		NativeLibClass = env_->FindClass("com/beloko/opengames/gzdoom2/NativeLib");
		swapBuffersMethod = env_->GetStaticMethodID(NativeLibClass, "swapBuffers", "()V");
	}
	env_->CallStaticVoidMethod(NativeLibClass, swapBuffersMethod);
}


//#define JAVA_FUNC(x) Java_com_beloko_opengames_gzdoom2_NativeLib_##x


#define JAVA_FUNC(x) Java_org_libsdl_app_NativeLib_##x

#define EXPORT_ME __attribute__ ((visibility("default")))

__attribute__((visibility("default"))) jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	LOGI("JNI_OnLoad");
	setTCJNIEnv(vm);
	return JNI_VERSION_1_4;
}

static int argc=1;
static const char * argv[32];

jint EXPORT_ME
JAVA_FUNC(init) ( JNIEnv* env,	jobject thiz,jstring graphics_dir,jint audio_rate,jobjectArray argsArray,jint lowRes,jstring game_path_ )
{
	env_ = env;
	android_audio_rate = audio_rate;

	argv[0] = "quake";
	int argCount = (env)->GetArrayLength( argsArray);
	LOGI("argCount = %d",argCount);
	for (int i=0; i<argCount; i++) {
		jstring string = (jstring) (env)->GetObjectArrayElement( argsArray, i);
		argv[argc] = (char *)(env)->GetStringUTFChars( string, 0);
		LOGI("arg = %s",argv[argc]);
		argc++;
	}


	static std::string game_path = (char *)(env)->GetStringUTFChars( game_path_, 0);
	static std::string graphics_path = (char *)(env)->GetStringUTFChars( graphics_dir, 0);

	LOGI("game_path = %s",game_path.c_str());

	setenv("HOME", game_path.c_str(),1);

	putenv("TIMIDITY_CFG=../timidity.cfg");

	chdir( game_path.c_str() );


	//Now done in java to keep context etc
	//SDL_SwapBufferPerformsSwap(false);

    mobile_init(android_screen_width, android_screen_height, graphics_path.c_str());
	PortableInit(argc,argv); //Never returns!!

	return 0;
}

void EXPORT_ME
JAVA_FUNC(setScreenSize) ( JNIEnv* env,	jobject thiz, jint width, jint height)
{
	android_screen_width = width;
	android_screen_height = height;
}

void EXPORT_ME
JAVA_FUNC(setTouchSettings) (JNIEnv *env, jobject obj,	jfloat alpha,jfloat strafe,jfloat fwd,jfloat pitch,jfloat yaw,int other)
{
//	setTouchSettings(alpha,strafe,fwd,pitch,yaw,other);
}


void EXPORT_ME
JAVA_FUNC(touchEvent) (JNIEnv *env, jobject obj,jint action, jint pid, jfloat x, jfloat y)
{
	//LOGI("TOUCHED");
	mobileGetTouchInterface()->processPointer(action,pid,x,y);
}

}