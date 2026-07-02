plugins {
    id("com.android.library")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "com.glancehud.service"
    compileSdk = 34

    defaultConfig {
        minSdk = 24  // Android 7.0 — matches the core library.
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
    // Opt-in add-on to the core library: pull this in only if you need the
    // overlay to survive the host app going to the background.
    implementation(project(":"))
    implementation("androidx.core:core-ktx:1.13.1")
}
