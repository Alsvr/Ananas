/*
 ****************************************************************************
 *  Copyright (c) 2015 Dark Guan <tickel.guan@gmail.com>                    *
 *	This file is part of Ananas.                                            *
 *                                                                          *
 *  Ananas is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, either version 3 of the License, or       *
 *  (at your option) any later version.                                     *
 *                                                                          *
 *  Ananas is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with Ananas.  If not, see <http://www.gnu.org/licenses/>.       *
 ****************************************************************************
*/
/*
 * stepper.cpp

 *
 *  Created on: 2015��12��14��
 *      Author: Dark
 */

#include "Ananas.h"
#include "QUN.h"
#include "Encoder.h"
#include "DA.h"

long pulseCount;

long encoderCount;

long error;
uint16_t errorAbs;

bool dirState;
bool enalbeState;

bool invDir;

bool enableDelay = false;

uint16_t delaytime;

//ʹ�ñȽ����ж�
QUN M;

//define
void pulse();
void stepastep();
void managerDalaytime(long error);
void mapThepin();

void initialStepper() {
	//set input pins
	//������Ҫ�õ������ʱ�� ��Ҫ���� ����
	pinMode(DIRPIN, INPUT);
	pinMode(STEPPIN, INPUT);
	pinMode(ENABLEPIN, INPUT);

	//set output pins
	pinMode(STEP_DIR, OUTPUT);
	pinMode(STEP_STEP, OUTPUT);
	pinMode(STEP_ENABLE, OUTPUT);

	//��ʼ���������
//	digitalWrite(STEP_ENABLE, LOW);
	//��step�˿���Ϊ��λ
	digitalWrite(STEP_STEP, HIGH);

	//add interrupt
	//TODO ��Ҫ�����λ���жϣ������ʼ���Ǹ߻��ǵ͵�ƽ
	M.attachInterrupt(pulse, M_FALLING); //D7
	//���½��ؼ���
	// M_CHANGE �����ŵ�ƽ�����ı�ʱ�������ж�
	// M_RISING �������ɵ͵�ƽ��Ϊ�ߵ�ƽʱ�������ж�
	// M_FALLING �������ɸߵ�ƽ��Ϊ�͵�ƽʱ�������ж�
	pulseCount = 0;
	//
	invDir = INV_DIR;

	delaytime = DELAYTIME;
	mapThepin();

}

//ֻ�ڵ��Ե�ʱ��ʹ��
void changeMotorDir() {
	invDir = !invDir;
}

void pulse() {
//����λ��������м���
//ͬʱʵʱ��������������
	noInterrupts();

	digitalWrite(STEP_ENABLE, LOW);

	dirState = digitalRead(DIRPIN);
	//�������
	if (dirState) {
		digitalWrite(STEP_DIR, invDir);
		pulseCount++;
		stepastep();
	} else {
		digitalWrite(STEP_DIR, !invDir);
		pulseCount--;
		stepastep();
	}
	interrupts();
}

//������ ת��һȦ �����· �������Ե�����������
//ʵ�ʲ�ʹ��
void step(bool dir) {
	if (dir) {
		digitalWrite(STEP_DIR, !invDir);
	} else {
		digitalWrite(STEP_DIR, invDir);
	}
	for (int i = 0; i < (MOTOR_STEPS_PER_CIRCLE * DRIVE_SUBDIVE); i++) {
		stepastep();
		delayMicroseconds(DELAYTIME / 2);
	}
}

void stepastep() {
	digitalWrite(STEP_STEP, LOW);
	digitalWrite(STEP_STEP, HIGH);
}

//��Ҫ������ENABLEPIN
void mapThepin() {
	enalbeState = digitalRead(ENABLEPIN);
	digitalWrite(STEP_ENABLE, enalbeState);
}

float getDestination() {
	return float(pulseCount / float(STEPS_PER_UNIT));
}

long getSteps() {
	return pulseCount;
}

long getError() {
	return error;
}

void managerDelay() {
	encoderCount = getEncodercount();
	error = (encoderCount * STRONG_PURSE_FACTOR - pulseCount);
	errorAbs = abs(error);

//manage the voltage
	managerVoltage(errorAbs);
	managerDalaytime(errorAbs);

	if (errorAbs >= STRONG_PURSE_FACTOR) {
		noInterrupts();
		digitalWrite(STEP_ENABLE, LOW);
		if (error > 0) {
			digitalWrite(STEP_DIR, !invDir);
			stepastep();
		} else {
			digitalWrite(STEP_DIR, invDir);
			stepastep();
		}
		interrupts();
		if (enableDelay)
			delayMicroseconds(delaytime);
	}
//	delayMicroseconds(1000);

}

void managerDalaytime(long error) {
	if (error < (MAXERROR)) {
		enableDelay = true;
		delaytime = DELAYTIME;
	} else {
		enableDelay = true;
		delaytime = MIN_DELAYTIME;	//����
	}
}

void changeDisableMotor() {
//	mapThepin();
	digitalWrite(STEP_ENABLE, HIGH);
}
