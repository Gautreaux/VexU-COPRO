#include "spencerPID.h"

#include "vexMessenger.h"
#include "vexController.h"

enum class VexColor{
    ANY = -1,
    NONE = 0,
    RED = 1,
    BLUE = 2
};

// class GoalState{
//     VexColor bottom;
//     VexColor middle;
//     VexColor top;
    
// public:
//     const VexColor& operator[](std::size_t idx) const;

//     // take the bottom and move down
//     VexColor pick(void);

//     //put a new ball
//     //  return true iff space to put the ball
//     bool put(const VexColor color);

//     // return true iff space to receive another ball
//     bool hasSpace(void) const;

//     // return the color that owns this goal
//     VexColor owner(void);

//     // return true iff this goal is owned by team color
//     //  passing VexColor::None always returns false
//     //  passing VexColor::Any returns true if a ball is in the goal
//     bool isOwned(const VexColor color);
// };

// class FieldState{
//     GoalState goals[9];
// };

// class representing a possible goal target
class GoalTarget{
public:
    int center_x;
    int center_y;
    int h;
    int w;
};

// class resenting a ball target
class VisionBall{
public:
    int ball_x;
    int ball_y;
    int radius;
};

// functions supporting programming skills:

// score in the goal if it is present in the frame
//  return true if scored successfully,
//      else return false
// function is non-preemptable
//  do not use inside a driver control loop
bool scoreGoalBlocking(void);

// score in the goal if it is present in the frame
//  return true if scored successfully,
//      else return false
// function is non-preemptable
//  do not use inside a driver control loop
bool grabBallBlocking(bool currentColor);

// for the best goal present in the frame
//  score in the goal
//  and attempt to descore the opponents
//  return true if the goal is owned by our team at the end
// function is non-preemptable
//  do not use inside a driver control loop
bool clearAndCycleGoalBlocking();