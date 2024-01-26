/**
******************************************************
* @file IO.h
* @brief Documentacion de Modulo Outlet wifi
* @author FabLab Lleida
* @author Jose Martinez Molero
* @author Julian Salas Bartolome
* @version 1.0
* @date Abril 2018
*
*
*******************************************************/
#ifndef IO_H
	#define IO_H

		#ifdef MiniD1_0
			int PinReset = 0;
			int PinLed = 2;
		#endif	
		#ifdef MiniD1_1
			int PinReset = 0;
			int PinLed = 2;
			int PinPir = 4;
		#endif	

#endif