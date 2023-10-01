# cpptf
this is very simply test util.

## how to
0. include header
 ```c++
 #include <Cpptf.hpp>
 ```  
1. set section name (option)
 ```c++
 cpptf::change_section("section1");
 ```
2. write test code
 ```c++
 // test pass when targetfunc() == 1.
 cpptf::isSame("test1", targetfunc(), 1);

 // test pass when throw any exception in lambda.
 cpptf::except_any("test2", ()[]{throwExceptionFunc();});
 ```
3. finally, write function about show result
 ```c++
 cpptf::complete();
 ```
4. show result
5. let's drink coffee.
