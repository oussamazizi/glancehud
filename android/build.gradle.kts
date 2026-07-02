plugins {
    id("com.android.library")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "com.glancehud"
    compileSdk = 34
    ndkVersion = "27.3.13750724"  // matches an installed NDK to avoid a download

    defaultConfig {
        minSdk = 24  // Android 7.0 — floor, not a target: runs on everything newer.

        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++20"
            }
        }
        // Ship one native lib per common CPU; the device installs only its own.
        ndk {
            abiFilters += listOf("arm64-v8a", "armeabi-v7a", "x86_64")
        }
        consumerProguardFiles("consumer-rules.pro")
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.30.5"  // matches the installed SDK CMake
        }
    }

    sourceSets["main"].java.srcDirs("src/main/kotlin")

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions {
        jvmTarget = "17"
    }

    buildTypes {
        release {
            isMinifyEnabled = false
        }
    }
}

dependencies {
}
