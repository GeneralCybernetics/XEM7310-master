use cmake;
use bindgen;
use std::env;
use std::path::PathBuf;

fn main() {
    let dst = cmake::Config::new("../rhxcpp").build();
    
      // Print the library search path
    println!("cargo:rustc-link-search=native={}/lib", dst.display());

    // Link the C++ static libraries
    println!("cargo:rustc-link-lib=static=rhxcontroller");
    println!("cargo:rustc-link-lib=static=rhxregisters");
    println!("cargo:rustc-link-lib=static=rhxdatablock");

    // Link the okFrontPanel dynamic library
    println!("cargo:rustc-link-search=native={}/../lib", env::current_dir().unwrap().display()); // Assuming okFrontPanel is in ../lib relative to current dir
    println!("cargo:rustc-link-lib=dylib=okFrontPanel");

    // Link the C++ standard library
    println!("cargo:rustc-link-lib=c++");

    let bindings = bindgen::Builder::default()
        .header("wrapper.hpp")
        .clang_arg("-xc++") // Treat header as C++
        .clang_arg("-std=c++20") // Specify C++ standard if needed
        .generate_inline_functions(true)
        .opaque_type("std::.*")
        // .blocklist_type("std::.*")
        // .opaque_type("std::memory_order")
        // .opaque_type("std::memory_order_relaxed")
        // .opaque_type("std::memory_order_consume")
        // .opaque_type("std::memory_order_acquire")
        // .opaque_type("std::memory_order_release")
        // .opaque_type("std::memory_order_acq_rel")
        // .opaque_type("std::memory_order_seq_cst")
        // .opaque_type("_Tp")
        // .opaque_type("_ValueType")
        // .opaque_type("rep")
        // .opaque_type("char_type")
        // .opaque_type("type_")
        // .opaque_type("size_type")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings.write_to_file(out_path.join("bindings.rs")).expect("Couldn't write bindings!");
}
