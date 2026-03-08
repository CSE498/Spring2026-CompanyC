/**
 * @file main.cpp
 * @author Jose Antonio Hernandez- Martinez
 */


#include "Memofunction.h"
#include <iostream>


using std::cout, std::endl;



int RandomMath (int x) {  /// inputs = 5, 7, 9, 10 // outputs = 17, 23, 29, 32

    x *=3;
    x +=2;
    ///std::cout << " Work " << std::endl;
    return x;
}

int main(){
    Memofunction Memo(RandomMath);
    Memo(5);
    Memo(5);
    Memo(7);
    Memo(9);
    Memo(9);
    Memo(10);

    std::cout << Memo(5) << std::endl;
    std::cout << Memo(5) << std::endl;
    std::cout << Memo(7) << std::endl;
    std::cout << Memo(9) << std::endl;
    std::cout << Memo(9) << std::endl;
    std::cout << Memo(10) << std::endl;
    return 1;

}