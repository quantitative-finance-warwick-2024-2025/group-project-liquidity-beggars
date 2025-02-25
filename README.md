[![Open in Codespaces](https://classroom.github.com/assets/launch-codespace-2972f46106e565e64193e422d61a12cf1da4916b45550586e14ef0a7c637dd04.svg)](https://classroom.github.com/open-in-codespaces?assignment_repo_id=18399851)
# IB9JHO Group Project
This is a blank template for your group to implement your project in the IB9JHO environment.

# Using CMake to build your main program:
When you add source files to the project which you want compiled and linked to your program, you need to add them to the CMakeLists.txt file at the root of the project. If you are getting LNK errors it is usually because you forgot to add one of your .cpp files

For example:
```
set(SRC_FILES ${CMAKE_SOURCE_DIR}/src/my_class1.cpp ${CMAKE_SOURCE_DIR}/src/my_class2.cpp)
add_executable(my_program ${SRC_FILES} ${CMAKE_SOURCE_DIR}/src/main.cpp)

```

Will create an executable called my_program by compiling my_class1.cpp, my_class2.cpp and main.cpp. Any cpp files you want to add to the project can be set in the first line.
To keep things simple, make sure you put all source and header files in the /src directory.

Note: Some functions from the standard math library may require that you link your executable explictly. This is usually only a problem on linux and is fixed by adding 
```
target_link_libraries(my_program m)
```
after add_executable

# Using CMake to add tests:

There is a seperate CMakeLists.txt file in the /tests directory for adding tests. For each test you would like to add to the testing suite you 
will add a cpp file which includes catch2 (see tests/my_test.cpp for an example).

Then in the CMakeLists.txt file:
```
add_executable(my_test ${SRC_FILES} my_test.cpp)
target_include_directories(my_test PUBLIC ${CMAKE_SOURCE_DIR}/extern/catch2 ${CMAKE_SOURCE_DIR}/src)
add_test(NAME my_test COMMAND my_test)
```

will create a test called my_test which will run the my_test.cpp file. Note that SRC_FILES is inherited from the main CMakeLists.txt file so in the case above your test has all the same source files as the main program.

# Using Git to manage your group:
It is suggested that each group member creates their own branch to prevent clashes. Each group member can create branches in github by clicking
'branches' when you are on the <> code tab of the repository. Each branch is independent from the main branch which will eventually contain your final project.

You can add the changes from your personal branch to the main branch using pull requests. You can create these in github, go to the pull requests tab and click new pull request. Then select the branch you want to merge into the main branch. It is recommended to then have the strongest programmer in the team review these pull requests and check your code can safely be merged without breaking anything.

# Submission
To submit, push your final project to your group's repository on GitHub on the main branch. Extract the contents of the repository to a zip file and submit it via my.wbs.
