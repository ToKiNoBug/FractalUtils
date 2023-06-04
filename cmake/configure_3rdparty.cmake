file(GLOB files "${CMAKE_SOURCE_DIR}/cmake/configure/*.cmake")

foreach(filename ${files})
    include(${filename})
endforeach(filename ${files})

unset(files)