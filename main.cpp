#include <iostream>
#include <fstream>

class NewClass {
public:
    int firstVar;
    int secondVar;

    NewClass(int firstVar, int secondVar);
};

NewClass::NewClass(int firstVar, int secondVar) : firstVar(firstVar), secondVar(secondVar) {
    std::cout << this->firstVar << " " << this->secondVar << std::endl;
}

int main() {
    NewClass myNewClass(7, 6);
    std::cout << myNewClass.firstVar << " " << myNewClass.secondVar << std::endl;
    return 0;
}
