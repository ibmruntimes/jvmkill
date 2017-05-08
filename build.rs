extern crate bindgen;

use std::env;
use std::path::PathBuf;

fn main() {
    // Tell cargo to tell rustc to link the system bzip2
    // shared library.
    //println!("cargo:rustc-link-lib=bz2");

    let java_include_directory = PathBuf::from(env::var("JAVA_HOME").unwrap()).join("include");

    let java_platform_include_directory = if cfg!(target_os = "macos") {
        "darwin"
    } else {
        "linux"
    };

    let bindings = bindgen::Builder::default()
        .no_unstable_rust()
        .header("jvmti_wrapper.h")
        .clang_arg(clang_include(&java_include_directory))
        .clang_arg(clang_include(&java_include_directory.join(java_platform_include_directory)))
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("jvmti_bindings.rs"))
        .expect("Couldn't write bindings!");
}

fn clang_include(path: &PathBuf) -> String {
    format!("-I/{}", path.to_str().unwrap())
}