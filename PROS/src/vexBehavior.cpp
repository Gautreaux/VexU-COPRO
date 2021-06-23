#include "vexBehavior.h"

// class GoalState{
//     VexColor bottom;
//     VexColor middle;
//     VexColor top;
    
// public:
//     const VexColor& operator[](std::size_t idx) const {
//         switch (idx)
//         {
//         case 0:
//             return bottom;
//         case 1:
//             return middle;
//         case 2:
//             return top;
//         default:
//             return VexColor::NONE;
//         }
//     }

//     VexColor pick(void){
//         if (bottom == VexColor::NONE){
//             return VexColor::NONE;
//         }

//         auto toReturn = bottom;
//         bottom = middle;
//         middle = top;
//         top = VexColor::None;
//         return toReturn;
//     }

//     bool put(const VexColor color){
//         if(hasSpace()){
//             if(bottom == VexColor::NONE){
//                 bottom = color;
//             }else if(middle == VexColor::NONE){
//                 middle = color;
//             }else{
//                 top = color;
//             }
//             return true;
//         }else{
//             return false;
//         }
//     }

//     bool hasSpace(void) const{
//         return top == VexColor::NONE;
//     }

//     VexColor owner(void) const{
//         if (top != VexColor::NONE){
//             return top;
//         }
//         else if(middle != VexColor::NONE){
//             return middle;
//         }else{
//             return bottom;
//         }
//     }

//     bool isOwned(const VexColor color) const{
//         return (color & owner());
//     }
// }


bool scoreGoalBlocking(void){
if(!VexMessenger::v_messenger->isConnected()){
        return false;
    }else{
        pros::lcd::print(LCD_MESSENGER_CONNECTED, "MESSENGER_CONNECTED");

        //see if there is a message to process

        //used to keep this loop from being greedy and blocking
        int msgCounter = 16;
        while(msgCounter-- && VexMessenger::v_messenger->readDataMessage(messageBuffer, messageLen, 0)){
            processMessage(messageBuffer, messageLen);
        }
    }

	int targetX = goalConst[0];
	int targetY = goalConst[1];
	int targetW = goalConst[2];
	int targetH = goalConst[3];

    if(targetX == 0 && targetY == 0){
		// no goal
        return false;
    }

    

}