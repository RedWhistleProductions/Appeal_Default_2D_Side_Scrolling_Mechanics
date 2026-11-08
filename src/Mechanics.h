#pragma once
#include <string>

extern "C"
{
    void Constructor();
    void Destructor();
    void Debug(bool Enable);
    void Set_Paths(std::string Resources);
    void Select(std::string Entity);
    void Enter(std::string Entity);
    void Exit(std::string Entity);
    void Update(std::string Entity);
    void Digital(std::string Entity, std::string Action);
    void Analog(std::string Entity, std::string Action, int Value);
    void Set(std::string Entity, std::string Key, std::string Value);
    std::string Get(std::string Entity, std::string Key);
    void Stand();
    void Stand_Analog(int Value);
    void Walk();
    void Walk_Analog(int Value);
    void Run();
    void Run_Analog(int Value);
    void Jump();
    void Jump_Analog(int Value);
    void Duck_Stand();
    void Duck_Stand_Analog(int Value);
    void Duck_Walk();
    void Duck_Walk_Analog(int Value);
    void Stop();
    void Stop_Analog(int Value);
    void Move_Left();
    void Move_Left_Analog(int Value);
    void Move_Right();
    void Move_Right_Analog(int Value);
}
