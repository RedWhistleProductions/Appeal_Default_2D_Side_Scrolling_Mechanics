#include "Mechanics.h"
#include <cmath>
#include <iostream>
#include <map>
#include <string>

namespace
{
    struct Entity_State
    {
        std::string Mode = "Ground";
        std::string Action = "Stand";
        std::string Ground_Action = "Stand";
        double Speed = 4;
        double Run_Multiplier = 2;
        double Duck_Multiplier = 0.5;
        double Jump_Speed = 6;
        double Gravity = 9.8;
        double Delta_Time = 1.0 / 60.0;
        double Ground_Height = 0;
        double Position_X = 0;
        double Position_Y = 0;
        double Velocity_X = 0;
        double Velocity_Y = 0;
        double Duck_Amount = 0;
        int Input = 0;
        bool Grounded = true;
        bool External_Physics = false;
    };

    std::map<std::string, Entity_State> Entities;
    std::string Selected_Entity;
    bool Debug_Output = false;

    Entity_State *Selected()
    {
        auto Found = Entities.find(Selected_Entity);
        return Found == Entities.end() ? nullptr : &Found->second;
    }

    void Act(const std::string &Action, int Value)
    {
        Entity_State *Entity = Selected();
        if(Entity == nullptr) return;
        // Input has already been mapped and dead-zone filtered by Input.
        if(Value < -100 || Value > 100) return;
        if(Action == "Stop") { Act("Stand", Value); return; }
        if(Action == "Move_Left") { Act("Walk", -Value); return; }
        if(Action == "Move_Right") { Act("Walk", Value); return; }
        if(Action != "Stand" && Action != "Walk" && Action != "Run" &&
           Action != "Jump" && Action != "Duck_Stand" && Action != "Duck_Walk") return;
        Entity->Input = Value;
        const double Amount = Value / 100.0;
        if(Action == "Jump")
        {
            if(Value == 0 || !Entity->Grounded) return;
            Entity->Velocity_Y = Entity->Jump_Speed * std::abs(Amount);
            Entity->Grounded = false;
            Entity->Duck_Amount = 0;
        }
        else
        {
            Entity->Duck_Amount = (Action == "Duck_Stand" || Action == "Duck_Walk")
                ? std::abs(Amount) : 0;
            double Multiplier = 0;
            if(Action == "Walk") Multiplier = 1;
            else if(Action == "Run") Multiplier = Entity->Run_Multiplier;
            else if(Action == "Duck_Walk") Multiplier = Entity->Duck_Multiplier;
            Entity->Velocity_X = Entity->Speed * Multiplier * Amount;
            Entity->Ground_Action = Value == 0 ? "Stand" : Action;
        }
        // Preserve the airborne action until landing; movement still controls X.
        if(Entity->Grounded) Entity->Action = Value == 0 ? "Stand" : Action;
        else Entity->Action = "Jump";
        if(Debug_Output)
            std::cout << "Mechanics " << Selected_Entity << " " << Action << " " << Value << '\n';
    }
}

extern "C" void Constructor() {}
extern "C" void Destructor() { Entities.clear(); Selected_Entity.clear(); }
extern "C" void Debug(bool Enable) { Debug_Output = Enable; }
extern "C" void Set_Paths(std::string) {}
extern "C" void Select(std::string Entity)
{
    Selected_Entity = Entities.count(Entity) ? Entity : "";
}
extern "C" void Enter(std::string Entity)
{
    Entities.try_emplace(Entity);
    Select(Entity);
}
extern "C" void Exit(std::string Entity)
{
    auto Found = Entities.find(Entity);
    if(Found != Entities.end())
    {
        Found->second.Velocity_X = 0;
        Found->second.Velocity_Y = 0;
        Found->second.Action = "Stand";
        Found->second.Ground_Action = "Stand";
        Found->second.Duck_Amount = 0;
    }
    if(Selected_Entity == Entity) Selected_Entity.clear();
}

// Digital exports have no arguments. Analog exports receive one mapped value.
extern "C" void Stand() { Act("Stand", 100); }
extern "C" void Stand_Analog(int Value) { Act("Stand", Value); }
extern "C" void Walk() { Act("Walk", 100); }
extern "C" void Walk_Analog(int Value) { Act("Walk", Value); }
extern "C" void Run() { Act("Run", 100); }
extern "C" void Run_Analog(int Value) { Act("Run", Value); }
extern "C" void Jump() { Act("Jump", 100); }
extern "C" void Jump_Analog(int Value) { Act("Jump", Value); }
extern "C" void Duck_Stand() { Act("Duck_Stand", 100); }
extern "C" void Duck_Stand_Analog(int Value) { Act("Duck_Stand", Value); }
extern "C" void Duck_Walk() { Act("Duck_Walk", 100); }
extern "C" void Duck_Walk_Analog(int Value) { Act("Duck_Walk", Value); }
extern "C" void Stop() { Act("Stop", 100); }
extern "C" void Stop_Analog(int Value) { Act("Stop", Value); }
extern "C" void Move_Left() { Act("Move_Left", 100); }
extern "C" void Move_Left_Analog(int Value) { Act("Move_Left", Value); }
extern "C" void Move_Right() { Act("Move_Right", 100); }
extern "C" void Move_Right_Analog(int Value) { Act("Move_Right", Value); }

extern "C" void Digital(std::string Entity, std::string Action)
{
    const std::string Previous_Selection = Selected_Entity;
    Select(Entity);
    if(Action == "Stand") Stand();
    else if(Action == "Walk") Walk();
    else if(Action == "Run") Run();
    else if(Action == "Jump") Jump();
    else if(Action == "Duck_Stand") Duck_Stand();
    else if(Action == "Duck_Walk") Duck_Walk();
    else if(Action == "Stop") Stop();
    else if(Action == "Move_Left") Move_Left();
    else if(Action == "Move_Right") Move_Right();
    Selected_Entity = Previous_Selection;
}
extern "C" void Analog(std::string Entity, std::string Action, int Value)
{
    const std::string Previous_Selection = Selected_Entity;
    Select(Entity);
    Act(Action, Value);
    Selected_Entity = Previous_Selection;
}
extern "C" void Update(std::string Entity)
{
    auto Found = Entities.find(Entity);
    if(Found == Entities.end()) return;
    Entity_State &State = Found->second;
    if(State.External_Physics) return;
    State.Position_X += State.Velocity_X * State.Delta_Time;
    if(State.Grounded) State.Position_Y = State.Ground_Height;
    if(!State.Grounded)
    {
        State.Position_Y += State.Velocity_Y * State.Delta_Time
            - 0.5 * State.Gravity * State.Delta_Time * State.Delta_Time;
        State.Velocity_Y -= State.Gravity * State.Delta_Time;
        if(State.Position_Y <= State.Ground_Height && State.Velocity_Y <= 0)
        {
            State.Position_Y = State.Ground_Height;
            State.Velocity_Y = 0;
            State.Grounded = true;
            State.Action = State.Ground_Action;
        }
    }
}
extern "C" void Set(std::string Entity, std::string Key, std::string Value)
{
    auto Found = Entities.find(Entity);
    if(Found == Entities.end()) return;
    Entity_State &State = Found->second;
    if(Key == "Mode") { State.Mode = Value; return; }
    if(Key == "External_Physics") { State.External_Physics = Value == "1" || Value == "True"; return; }
    if(Key == "Grounded")
    {
        if(Value == "1" || Value == "0")
        {
            State.Grounded = Value == "1";
            if(State.Grounded) { State.Velocity_Y = 0; if(!State.External_Physics) State.Position_Y = State.Ground_Height; State.Action = State.Ground_Action; }
            else State.Action = "Jump";
        }
        return;
    }
    try
    {
        std::size_t Used = 0;
        const double Number = std::stod(Value, &Used);
        if(Used != Value.size() || !std::isfinite(Number)) return;
        if(Key == "Velocity_X") State.Velocity_X = Number;
        else if(Key == "Velocity_Y") State.Velocity_Y = Number;
        else if(Key == "Speed" && Number >= 0) State.Speed = Number;
        else if(Key == "Run_Multiplier" && Number >= 0) State.Run_Multiplier = Number;
        else if(Key == "Duck_Multiplier" && Number >= 0) State.Duck_Multiplier = Number;
        else if(Key == "Jump_Speed" && Number >= 0) State.Jump_Speed = Number;
        else if(Key == "Gravity" && Number >= 0) State.Gravity = Number;
        else if(Key == "Delta_Time" && Number > 0) State.Delta_Time = Number;
        else if(Key == "Ground_Height") State.Ground_Height = Number;
        else if(Key == "Position_X") State.Position_X = Number;
        else if(Key == "Position_Y") State.Position_Y = Number;
    }
    catch(const std::exception &) { /* Invalid settings leave entity state unchanged. */ }
}
extern "C" std::string Get(std::string Entity, std::string Key)
{
    auto Found = Entities.find(Entity);
    if(Found == Entities.end()) return "";
    const Entity_State &State = Found->second;
    if(Key == "Mode") return State.Mode;
    if(Key == "External_Physics") return State.External_Physics ? "1" : "0";
    if(Key == "Orientation") return "2D_Side_Scrolling";
    if(Key == "Action") return State.Action;
    if(Key == "Grounded") return State.Grounded ? "1" : "0";
    if(Key == "Input") return std::to_string(State.Input);
    if(Key == "Speed") return std::to_string(State.Speed);
    if(Key == "Run_Multiplier") return std::to_string(State.Run_Multiplier);
    if(Key == "Duck_Multiplier") return std::to_string(State.Duck_Multiplier);
    if(Key == "Jump_Speed") return std::to_string(State.Jump_Speed);
    if(Key == "Gravity") return std::to_string(State.Gravity);
    if(Key == "Delta_Time") return std::to_string(State.Delta_Time);
    if(Key == "Ground_Height") return std::to_string(State.Ground_Height);
    if(Key == "Position_X") return std::to_string(State.Position_X);
    if(Key == "Position_Y") return std::to_string(State.Position_Y);
    if(Key == "Velocity_X") return std::to_string(State.Velocity_X);
    if(Key == "Velocity_Y") return std::to_string(State.Velocity_Y);
    if(Key == "Duck_Amount") return std::to_string(State.Duck_Amount);
    return "";
}
