#include "lc3_module/lc3.hpp"

using namespace std;


int main (int argc, char** argv)
{

    // @Loading args
    if (argc < 2)
    {
        cout << "lc3: image file don't founded" << endl;
        exit(2);
    }


    
    lc3vm lc;



    lc.run();
    return 0;
}