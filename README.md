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
----
## using macro
define`CPPTF_MACRO_TEST` to enable macro.
you can use this to test in anywhere.
```c++
#define CPPTF_MACRO_TEST
#include <Cpptf.hpp>

CPPTF_TEST([](){
    // this code execute in call cpptf::complete();
    cpptf::isSame("test1", targetfunc(), 1);
})
```

if define `CPPTF_TEST_CATCH`, not caught exception will be notice in result;
```
$ ./example_test
===========================================
       Unchaught exception is occur!
===========================================
occur at : /home/folosuru/cpptf/example.cpp:63
 - what(): invalid vector subscript
 
 
===========================================
stats        section / failed        passed
===========================================
 [o] some test                        [3/3]
===========================================
 [o] total                            [3/3]

```