#include <iostream>
#include <array>


int fill_up_boxes (int tp, int ppb, int sb, int mb, int lb, int xlb, int sp, int mp, int lp, int xlp) // Fill up boxes function prototype
{
    int tb, fp=tp;                                                                                    // Initialising total no. of boxes (tb) and assigning it to final no. of parts (fp)
    std::array <int ,4> x{0,1,2,3};                                                                   // Initialising an array of 4 elements          
    int fsb=sb, fmb=mb, flb=lb, fxlb=xlb;

        tb = sb+mb+lb+xlb;                                                                            // Calculating the total no. of boxes
    
    while((xlb*xlp)>tp)                                                                               // While loop for checking if the total no.of parts is greater than the product of no.of boxes and the parts in that box
    {
        xlb=xlb-1;
    }

        tp=tp-(xlb*xlp);                                                                              // Calculating the remaining parts 
        x.at(3)=tp; 
        tb=tb-xlp;                                                                                    // Calculating the boxes that have been used
    
    while((lb*lp)>tp)                                                                                 // While loop for checking if the total no.of parts is greater than the product of no.of boxes and the parts in that box
    {
        lb=lb-1;
    }

        tp=tp-(lb*lp);                                                                                // Calculating the remaining parts
        x.at(2)=tp;
        tb=tb-lb;                                                                                     // Calculating the boxes that have been used
    
    while((mb*mp)>tp)                                                                                 // While loop for checking if the total no.of parts is greater than the product of no.of boxes and the parts in that box
    {
        mb=mb-1;
    }

        tp=tp-(mb*mp);                                                                                // Calculating the remaining parts
        x.at(1)=tp;
        tb=tb-mb;                                                                                     // Calculating the boxes that have been used

    while((sb*sp)>tp)                                                                                 // While loop for checking if the total no.of parts is greater than the product of no.of boxes and the parts in that box
    {
        sb=sb-1;
    }

        tp=tp-(sb*sp);                                                                                // Calculating the remaining parts
        x.at(0)=tp;
        tb=tb-sb;                                                                                     // Calculating the boxes that have been used


    std::cout << "Boxes that can be built with " << fp << " Pegs \n";
    std::cout << "---------------------------------------" << "\n";
    std::cout << "XL Box ("<< fxlb << "x" << xlp << " max) :  "<< xlb <<" -- remaining parts: " << x.at(3) << "\n";
    std::cout << "L Box ("<< flb << "x" << lp <<  "max): "<< lb <<" -- remaining parts: " << x.at(2) << "\n";
    std::cout << "M Box (" << fmb << "x" << mp << "max): "<< mb <<" -- remaining parts: " << x.at(1) << "\n";
    std::cout << "S Box (" << fsb << "x" << sp << "max): "<< sb <<" -- remaining parts: " << x.at(0) << "\n";
    
        return 1;
}
int main()
{
	int tp, ppb, sb, mb, lb, xlb, sp, mp, lp, xlp;

    std::cout << "How many parts in total? " << "\n";
    std::cin >> tp; 

while(tp<0)
{
    std::cout << "Please enter a positive integer: " ;
    std::cin >> tp;
}


    do{
        if(tp>0)
        {
            std::cout << "How many boxes S/M/L/XL?" << "\n"; 
            std::cin >> sb >> mb >> lb >> xlb;
            std::cout << "How many parts per box?" << "\n";
            std::cin >> sp >> mp >> lp >> xlp; 
        }
        
        else
        std::cout << "Enter correct number of parts: " ;
    }
    while(tp<0);

    int r= fill_up_boxes(tp,ppb,sb,mb,lb,xlb,sp,mp,lp,xlp);

}

