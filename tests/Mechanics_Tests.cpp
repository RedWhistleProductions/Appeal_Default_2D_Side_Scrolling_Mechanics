#include "../src/Mechanics.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

void Check(bool Condition, const char *Message)
{
    if(!Condition) throw std::runtime_error(Message);
}
double Number(const std::string &Entity, const std::string &Key)
{
    return std::stod(Get(Entity, Key));
}
void Near(double Actual, double Expected)
{
    Check(std::abs(Actual - Expected) < 0.00001, "Unexpected movement value");
}

int main()
{
    Constructor();
    Walk(); // No selected entity: no accidental entity creation.
    Check(Get("", "Action").empty(), "Action created an entity");
    Enter("Player");
    Set("Player", "Speed", "10");
    Walk(); Near(Number("Player", "Velocity_X"), 10);
    Run(); Near(Number("Player", "Velocity_X"), 20);
    Duck_Walk(); Near(Number("Player", "Velocity_X"), 5);
    Duck_Stand(); Near(Number("Player", "Velocity_X"), 0);
    Near(Number("Player", "Duck_Amount"), 1);
    Stand(); Near(Number("Player", "Duck_Amount"), 0);
    Move_Left(); Near(Number("Player", "Velocity_X"), -10);
    Move_Right(); Near(Number("Player", "Velocity_X"), 10);
    Move_Left_Analog(-50); Near(Number("Player", "Velocity_X"), 5);
    Move_Right_Analog(-50); Near(Number("Player", "Velocity_X"), -5);
    Stop(); Near(Number("Player", "Velocity_X"), 0);
    Walk(); Stop_Analog(0); Near(Number("Player", "Velocity_X"), 0);
    Check(Get("Player", "Orientation") == "2D_Side_Scrolling", "Wrong orientation");

    for(int Value : {-100, -50, -1, 0, 1, 50, 100})
    {
        Walk_Analog(Value); Near(Number("Player", "Velocity_X"), 10 * Value / 100.0);
        Run_Analog(Value); Near(Number("Player", "Velocity_X"), 20 * Value / 100.0);
        Duck_Walk_Analog(Value); Near(Number("Player", "Velocity_X"), 5 * Value / 100.0);
        Near(Number("Player", "Duck_Amount"), std::abs(Value / 100.0));
        Duck_Stand_Analog(Value); Near(Number("Player", "Velocity_X"), 0);
        Stand_Analog(Value); Near(Number("Player", "Velocity_X"), 0);
    }
    Near(Number("Player", "Speed"), 10); // Analog input preserves configured speed.
    Walk(); Walk_Analog(101); Near(Number("Player", "Velocity_X"), 10);
    Analog("Player", "Unknown", 50); Near(Number("Player", "Velocity_X"), 10);
    Set("Player", "Speed", "nan"); Set("Player", "Speed", "bad");
    Set("Player", "Speed", "-1"); Near(Number("Player", "Speed"), 10);

    Enter("NPC"); Set("NPC", "Speed", "2"); Run();
    Near(Number("NPC", "Velocity_X"), 4);
    Near(Number("Player", "Velocity_X"), 10);
    Select("Player"); Stand();
    Near(Number("NPC", "Velocity_X"), 4);
    Set("Player", "Delta_Time", "0.1"); Walk(); Update("Player");
    Near(Number("Player", "Position_X"), 1);
    Stand(); Jump_Analog(0); Check(Get("Player", "Grounded") == "1", "Zero jumped");
    Jump_Analog(50); Near(Number("Player", "Velocity_Y"), 3);
    Jump(); Near(Number("Player", "Velocity_Y"), 3); // No double jump.
    Update("Player"); Check(Number("Player", "Position_Y") > 0, "Jump did not move");
    for(int Step = 0; Step < 30; ++Step) Update("Player");
    Check(Get("Player", "Grounded") == "1", "Jump did not land");
    Near(Number("Player", "Position_Y"), 0);
    Jump(); Near(Number("Player", "Velocity_Y"), 6);
    Set("Player", "Grounded", "1"); Jump_Analog(-100);
    Near(Number("Player", "Velocity_Y"), 6);

    Exit("Player"); Walk(); Near(Number("Player", "Velocity_X"), 0);
    Enter("Player"); Near(Number("Player", "Speed"), 10);
    Digital("NPC", "Walk"); Near(Number("NPC", "Velocity_X"), 2);
    Analog("NPC", "Walk", -50); Near(Number("NPC", "Velocity_X"), -1);
    Stand(); Near(Number("Player", "Velocity_X"), 0);
    Near(Number("NPC", "Velocity_X"), -1); // Explicit calls preserve selection.
    Destructor(); Check(Get("Player", "Mode").empty(), "Destructor retained entities");
    std::cout << "Default Mechanics tests passed\n";
}
