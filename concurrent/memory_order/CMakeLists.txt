set(release_acquire_SOURCE_FILES
  release_acquire.cpp
)
add_executable(release_acquire ${release_acquire_SOURCE_FILES})

set(release_consume_SOURCE_FILES
  release_consume.cpp
)
add_executable(release_consume ${release_consume_SOURCE_FILES})


add_executable(relaxed)
target_sources(relaxed PRIVATE relaxed.cpp)