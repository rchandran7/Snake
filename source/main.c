/*	Author: Rohit Chandran
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Final Project
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <shiftregister.h>
#include <timer.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif


void ADC_init() {
  ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
  ADMUX = (1<<REFS0) | ADMUX;
  ADCSRA = (1<<ADSC) | ADCSRA;
}
uint16_t adc_read(uint8_t channel) {
  channel = channel & 0x07;
  ADMUX = (ADMUX&0xF8) | channel;
  ADCSRA = ADCSRA | (1<<ADSC);
  while(ADCSRA&(1<<ADSC));
  return ADC;
}
unsigned char joyValue = 0x05;
unsigned char x;
unsigned char y;
enum Joystick {XY, Set};
int JoystickTick(int state) {
  switch(state) {
    case XY:
      x = adc_read(1);
      y = adc_read(0);
      state = Set;
      break;
    case Set:
      state = XY;
      break;
    default:
      state = XY;
      break;
  }
  switch(state) {
    case XY:
      joyValue = 5;
      break;
    case Set:
      if(y>=190) {
        joyValue = 1;
      }
      else if(y<=15) {
        joyValue = 2;
      }
      else if(x<=15) {
        joyValue = 3;
      }
      else if(x>=190) {
        joyValue = 4;
      }
      else {
        joyValue = 0;
      }
      break;
    default:
      break;
  }
  return state;
}

void eepromWrite(unsigned int uiAddress, unsigned char ucData) {
  while(EECR & (1<<EEPE));
  EEAR = uiAddress;
  EEDR = ucData;
  EECR |= (1<<EEMPE);
  EECR |= (1<<EEPE);
}

unsigned char eepromRead(unsigned int uiAddress) {
 while(EECR & (1<<EEPE));
 EEAR = uiAddress;
 EECR |= (1<<EERE);
 return EEDR;
}

unsigned char map[40];
unsigned char snakePos[40];

unsigned char getMap(int row, int column) {
  if(column>=8 || row>=5) {
    return 0;
  }
  else {
    return map[(8*row) + column];
  }
}
void setMap(int row, int column, unsigned char position) {
  map[(8*row) + column] = position;
}
unsigned char getSnakePosition(int row, int column) {
  if(column>=8 || row>=5) {
    return 0;
  }
  else {
    return snakePos[(8*row) + column];
  }
}
void setSnakePosition(int row, int column, unsigned char position) {
  snakePos[(8*row) + column] = position;
}
unsigned char snakeRow = 2;
unsigned char snakeColumn = 4;
unsigned char bRow = 0x01;
unsigned char bColumn = 0x01;
unsigned char block = 0x00;
unsigned char dead = 0x00;
unsigned char length = 0x01;
unsigned char goRow;
unsigned char goColumn;
void updateSnake(int row, int column, int length) {
  if(length==1) {
    setSnakePosition(row, column, 0);
    return;
  }
  if(getSnakePosition(row+1, column) == (length-1)) {
    updateSnake(row+1, column, length-1);
  }
  else if(getSnakePosition(row-1, column) == (length-1)) {
    updateSnake(row-1, column, length-1);
  }
  else if(getSnakePosition(row, column+1) == (length-1)) {
    updateSnake(row, column+1, length-1);
  }
  else if(getSnakePosition(row, column-1) == (length-1)) {
    updateSnake(row, column-1, length-1);
  }
  setSnakePosition(row, column, getSnakePosition(row, column)-1);
}
enum SnakeDirection {FindDir, Go};
int SnakeDirectionTick(int state) {
  switch(state) {
    case FindDir:
      state = Go;
      break;
    case Go:
      state = Go;
      break;
    default:
      state = FindDir;
      break;
  }
  switch(state) {
    case FindDir:
      //LCD
      setSnakePosition(snakeRow, snakeColumn, 0x01);
      break;
    case Go:
      switch(joyValue) {
        case 1:
          if(snakeRow==4) {
            dead = 1;
            return state;
          }
          goRow = snakeRow + 1;
          goColumn = snakeColumn;
          break;
        case 2:
          if(snakeRow==0) {
            dead = 1;
            return state;
          }
          goRow = snakeRow - 1;
          goColumn = snakeColumn;
          break;
        case 3:
          if(snakeColumn == 0) {
            dead = 1;
            return state;
          }
          goRow = snakeRow;
          goColumn = snakeColumn - 1;
          break;
        case 4:
          if(snakeColumn == 7) {
            dead = 1;
            return state;
          }
          goRow = snakeRow;
          goColumn = snakeColumn + 1;
          break;
        default:
          goRow = snakeRow;
          goColumn = snakeColumn;
          break;
      }
      if((getSnakePosition(goRow, goColumn)&&joyValue)<5) {
        dead = 1;
        return state;
      }
      if((block&&goRow) == (bRow&&goColumn) == bColumn) {
        setSnakePosition(goRow, goColumn, ++length);
       //LCD
        block = 0;
      } else {
        if(joyValue <= 4) {
          setSnakePosition(goRow, goColumn, ++length);
          updateSnake(goRow, goColumn, length++);
        }
      }
      snakeRow = goRow;
      snakeColumn = goColumn;
    default:
      break;
  }
  return state;
}

unsigned char eaten = 0x00;
enum RandomBlocks{BlockStart, Generate, Eat};
int RandomBlocksTick(int state) {
  switch(state) {
    case BlockStart:
      state = Generate;
      break;
    case Generate:
      if(block==1) {
        state = Eat;
      }
      else {
        state = Generate;
      }
      break;
    case Eat:
      if(block==0) {
        state = Generate;
      }
      else {
        state = Eat;
      }
      break;
    default:
      state = BlockStart;
      break;
  }
  switch(state) {
    case BlockStart:
      break;
    case Generate:
      eaten = 0x00;
      do {
        int blockRow=-1;
        int blockColumn=-1;
        blockRow = rand()%5;
        blockColumn = rand()%8;
        if (getSnakePosition(blockRow, blockColumn)) { continue; }
        bRow = blockRow;
        bColumn = blockColumn;
        block = 1;
        eaten = 1;
      } while(eaten==0);
      break;
    case Eat:
      break;
    default:
      break;
  }
  return state;
}
unsigned char tmpC = 0x00;
unsigned char tmpD = 0xFF;
unsigned char tempColumn = 0x00;
unsigned char calc;
unsigned char highscore = 0x00;
unsigned char number;
unsigned char num;
enum SnakeMatrix{MatrixStart, Increase, Game, Over};
int SnakeMatrixTick(int state) {
  switch(state) {
    case MatrixStart:
      state = Increase;
      break;
    case Increase:
      if(dead==1) {
        state = Over;
      }
      else {
        state = Game;
      }
      break;
    case Game:
     if(dead==1) {
      state = Over;
     }
     else {
      state = Increase;
     }
     break;
    case Over:
      if(dead==0) {
        if((~PINA&0x80)==0x80) {
          state = Increase;
        }
      }
      break;
    default:
      state = MatrixStart;
      break;
  }
  switch(state) {
    case MatrixStart:
      break;
    case Increase:
      if(block) {
        setMap(bRow, bColumn, 1);
      }
      unsigned char temp = 0x02;
      for(int i=0; i<5; i++) {
        for(int j=0; j<8; j++) {
          if(getSnakePosition(i, j) >= 1) {
            temp = 1;
          }
          else { temp = 0; }
          setMap(i, j, temp);
        }
      }
      break;
    case Game:
      if(tempColumn>=8) {
        tempColumn=0;
      }
      number = 1;
      for(int k=0; k<tempColumn; k++) { number=number*2; }
      tmpC = number;
      calc=0;
      for(int i=0; i<5; i++) {
        num=1;
        for(int k=0; k<i; k++) {
          num = num*2;
        }
        if(getMap(i, tempColumn)==1) { calc=calc+num; }
      }
      tmpD=calc;
      row_data(tmpC);
      column_data(~tmpD);
      tempColumn++;
      break;
    case Over:
      if(length > highscore) {
  	eepromWrite(0x1A, length);
 	 highscore = eepromRead(0x1A);
  	PORTB = highscore;
      }
      else {
  	PORTB = highscore;
      }
      tmpC=0x00;
      tmpD=0xFF;
      row_data(tmpC);
      column_data(~tmpD);
      length=1;
      snakeRow=2;
      snakeColumn=4;
      dead=0;
      block=0;
      joyValue=5;
      for(unsigned long int i=0; i<40; i++) {
        map[i]=0;
        snakePos[i]=0;
      }
      setSnakePosition(snakeRow, snakeColumn, 1);
      break;
    default:
      break;
  }
  return state;
}
typedef struct task{
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);
} task;

int main(void) {
  DDRA = 0x00; PORTA = 0xFF;
  DDRB = 0xFF; PORTB = 0x00;
  DDRC = 0xFF; PORTB = 0x00;
  DDRD = 0xFF; PORTB = 0x00;

  srand(time(NULL));

  static task task1, task2, task3, task4;
  task *tasks[] = {&task1, &task2, &task3, &task4};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
  const char start = -1;

  task1.state = start;
  task1.period = 50;
  task1.elapsedTime = task1.period;
  task1.TickFct = &JoystickTick;

  task2.state = start;
  task2.period = 300;
  task2.elapsedTime = task2.period;
  task2.TickFct = &SnakeDirectionTick;

  task3.state = start;
  task3.period = 600;
  task3.elapsedTime = task3.period;
  task3.TickFct = &RandomBlocksTick;

  task4.state = start;
  task4.period = 1;
  task4.elapsedTime = task4.period;
  task4.TickFct = &SnakeMatrixTick;

  unsigned short i;
  TimerSet(1);
  for(i=0; i<40; i++) {
    map[i]=0;
    snakePos[i]=0;
  }
  TimerOn();

  while(1) {
    for(i = 0; i < numTasks; i++) {
      if (tasks[i]->elapsedTime == tasks[i]->period) {
        tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
        tasks[i]->elapsedTime = 0;
      }
      tasks[i]->elapsedTime += 1;
    }
    while(!TimerFlag);
    TimerFlag = 0;
  }
  
}
