/**
 * This file is part of the Caretaker Home Automation System
 *
 * Copyright 2011-2015 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the the MIT License
 * You find a copy of license in the root directory of this project
 */

#ifndef _MESSAGES_H
#define _MESSAGES_H

/** General messages */

#define MSG_INVALID             0
#define MSG_REGISTER_REQUEST    1
#define MSG_REGISTER_RESPONSE   2
#define MSG_PING_REQUEST        3
#define MSG_PING_RESPONSE       4
#define MSG_SWITCH_WRITE        5
#define MSG_SWITCH_READ         6
#define MSG_SWITCH_STATE        7
#define MSG_RGB_WRITE           8
#define MSG_RGB_READ            9
#define MSG_RGB_STATE           10
#define MSG_PWM_WRITE           11
#define MSG_PWM_READ            12
#define MSG_PWM_STATE           13

/** Value write modes */

#define WRITE_DEFAULT            0
#define WRITE_ABSOLUTE           1
#define WRITE_INCREMENT          2
#define WRITE_INCREMENT_DEFAULT  3
#define WRITE_DECREMENT          4
#define WRITE_DECREMENT_DEFAULT  5
#define WRITE_TOGGLE             6

#endif /* _MESSAGES_H */
