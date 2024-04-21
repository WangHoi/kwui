#pragma once
#include "ResourceProviderInterface.h"
#include <unordered_map>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

namespace base {

class ResourceAndroid : public ResourceProviderInterface {
public:
	static std::unique_ptr<ResourceAndroid> createFromJava(JNIEnv* env, jobject asset_manager);
	~ResourceAndroid() override;
	// 通过 ResourceProviderInterface 继承
	absl::optional<ResourceItem> findItem(const wchar_t* path) const override;

private:
	void init(JNIEnv* env, jobject asset_manager);
	AAssetManager* manager_ = nullptr;
	mutable std::unordered_map<std::wstring, ResourceItem> cache_;
};

}