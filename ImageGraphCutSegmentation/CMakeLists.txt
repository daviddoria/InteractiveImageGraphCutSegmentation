add_subdirectory(Kolmogorov)

# Create graph library
add_library(ImageGraphCut ImageGraphCut.cxx)
target_link_libraries(ImageGraphCut KolmogorovMaxFlow)

ADD_EXECUTABLE(NonInteractive NonInteractive.cpp)
TARGET_LINK_LIBRARIES(NonInteractive
${VTK_LIBRARIES} ${ITK_LIBRARIES}
ImageGraphCut
Mask)
