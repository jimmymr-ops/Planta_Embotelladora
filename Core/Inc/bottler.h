/*
 * bottler.h
 *
 *  Created on: May 22, 2025
 *      Author: jaime
 */

#ifndef INC_BOTTLER_H_
#define INC_BOTTLER_H_

#include <stdint.h>

#define EncenderLedAzul     encenderLed('A')                      /*Se definen las acciones fijas*/
#define EncenderLedNaranja  encenderLed('N')                           /*por comodidad y claridad*/
#define EncenderLedVerde    encenderLed('V')
#define EncenderLedRojo     encenderLed('R')
#define ApagarLedAzul         apagarLed('A')
#define ApagarLedNaranja      apagarLed('N')
#define ApagarLedVerde        apagarLed('V')
#define ApagarLedRojo         apagarLed('R')

#define MotorEmbotelladoraON conveyorForward(1)
#define MotorTaponadoraON    conveyorForward(2)
#define MotorEmpaquetadoraON conveyorForward(3)
#define MotorEmbotelladoraOFF   conveyorStop(1)
#define MotorTaponadoraOFF      conveyorStop(2)
#define MotorEmpaquetadoraOFF   conveyorStop(3)

#define AbrirValvula               valveOpen()
#define CerrarValvula             valveClose()

void bottlerInit(void);                                         /*Todas las funciones principales*/
void conveyorForward(uint8_t motor);
void conveyorStop(uint8_t motor);
void valveOpen(void);
void valveClose(void);
void servoAlternate(void);
void taponBajar(uint8_t taponadora);
void taponSubir(uint8_t taponadora);
void encenderLed(uint8_t led);
void apagarLed(uint8_t led);
void controlDisplay(uint8_t numero);

#endif /* INC_BOTTLER_H_ */
