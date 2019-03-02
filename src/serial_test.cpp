#include "serial_test.h"

void PrintCommState(const DCB & dcb)
{
_tprintf(TEXT("\nBaudRate = %d, ByteSize = %d, "), dcb.BaudRate, dcb.ByteSize);
switch (dcb.Parity)
	{
	case 0:
		_tprintf(TEXT("Parity = None, "));
		break;
	case 1:
		_tprintf(TEXT("Parity = Odd, "));
		break;
	case 2:
		_tprintf(TEXT("Parity = Even, "));
		break;
	case 3:
		_tprintf(TEXT("Parity = Mark, "));
		break;
	case 4:
		_tprintf(TEXT("Parity = Space, "));
		break;
	default:
		fprintf(stderr, "Wrong parity parameter!, ");
		break;
	} /* dcb.Parity */

switch (dcb.StopBits)
	{
	case 0:
		_tprintf(TEXT("StopBits = 1\n"));
		break;
	case 1:
		_tprintf(TEXT("StopBits = 1.5\n"));
		break;
	case 2:
		_tprintf(TEXT("StopBits = 2\n"));
		break;
	default:
		fprintf(stderr, "Wrong StopBits parameter!\n");
		break;
	} /* dcb.StopBits */
}

void SetCommParameters(DCB & dcb)
	{
	_tprintf(TEXT("\n Set BaudRate: "));
	_tscanf(TEXT("%d"), &dcb.BaudRate);
	if ((dcb.BaudRate > 256000) || ((dcb.BaudRate != 110) && (dcb.BaudRate != 56000) && (dcb.BaudRate != 128000) && (dcb.BaudRate != 256000) && (dcb.BaudRate % 75) && (dcb.BaudRate % 3)))
		fprintf(stderr, "Wrong BaudRate parameter!\n");

	_tprintf(TEXT("\n Set ByteSize: "));
	_tscanf(TEXT("%hhd"), &dcb.ByteSize);
	if ((dcb.ByteSize != 5) && (dcb.ByteSize != 6) && (dcb.ByteSize != 7) && (dcb.ByteSize != 8))
		fprintf(stderr, "Wrong ByteSize parameter!\n");

	_tprintf(TEXT("\n Set Parity(0-4 = None, Odd, Even, Mark, Space): "));
	_tscanf(TEXT("%hhd"), &dcb.Parity);
	if ((dcb.Parity != 0) && (dcb.Parity != 1) && (dcb.Parity != 2) && (dcb.Parity != 3) && (dcb.Parity != 4))
		fprintf(stderr, "Wrong Parity parameter!\n");

	_tprintf(TEXT("\n Set StopBits(1, 1.5, 2): "));
	float stop_bits = 0;
	fscanf(stdin, "%f", &stop_bits);
	if (stop_bits == 1)
		dcb.StopBits = ONESTOPBIT;
	else if (stop_bits == 1.5)
		dcb.StopBits = ONE5STOPBITS;
	else if (stop_bits == 2)
		dcb.StopBits = TWOSTOPBITS;
	else
		fprintf(stderr, "Wrong StopBits parameter!\n");
	}