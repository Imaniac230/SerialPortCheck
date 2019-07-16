#include "serial_test.h"

typedef struct SendData {
	HANDLE hSerial = NULL;
	const char* bytes_to_Send = nullptr;
	DWORD bytes_written = 0;
	size_t index = 0;
	OVERLAPPED oW = { 0, };
	bool port_dissapeared = false;
	DWORD total_bytes_written = 0;
	size_t char_count = 0;
	DWORD purgeFlags = 0;
	bool result_write = false;
	}SENDDATA, *PSENDDATA;

unsigned int __stdcall Sendthread(void* aSData);
bool end_thread = false;
bool interrupted = false;
bool sync_event = false;
bool sync_complete = false;

int _tmain(int argc, TCHAR *argv[])
	{
	PSENDDATA pSData;
	HANDLE hWriteThread;
	// Declare variables and structures
	HANDLE hSerial;
	DCB dcbSerialParams = { 0, };
	COMMTIMEOUTS timeouts = { 0, };
	OVERLAPPED oR = { 0, }, oW = { 0, };
	bool port_dissapeared = false, port_noexist = false, timer_start = false;
	auto start = std::chrono::high_resolution_clock::now();
	auto stop = std::chrono::high_resolution_clock::now();
	auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
	std::string new_port;

	do
		{
		new_port = "0";
		port_dissapeared = false;
		port_noexist = false;

		std::wcout << "Type the name of the serial port: ";
		std::wstring port_num;	std::wcin >> port_num;

		const TCHAR *pcCommPort = port_num.c_str();
		std::wcout << "Opening serial port " << pcCommPort << "...";

		//  Open a handle to the specified com port.
		hSerial = CreateFile(pcCommPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

		if (hSerial == INVALID_HANDLE_VALUE)
			{
			std::wcerr << "\aCreateFile failed with error " << GetLastError() << std::endl;
			port_dissapeared = true;
			port_noexist = true;
			}
		if (!port_dissapeared)
			std::wcout << "CreateFile exited OK!" << std::endl;
		/*
				// Set the event mask.
				if(!port_dissapeared && !SetCommMask(hSerial, EV_CTS | EV_DSR))
					{
					std::wcerr << "\aSetCommMask failed with error " << GetLastError() << std::endl;
					port_dissapeared = true;
					}
		*/


		//  Initialize the DCB structure.
		SecureZeroMemory(&dcbSerialParams, sizeof(DCB));
		dcbSerialParams.DCBlength = sizeof(DCB);

		//  Build on the current configuration by first retrieving all current settings.
		if (!port_dissapeared && !GetCommState(hSerial, &dcbSerialParams))
			{
			std::wcerr << "\aGetCommState failed with error " << GetLastError() << std::endl;
			port_dissapeared = true;
			}

		std::string change_params("0");
		if (!port_dissapeared)
			{
			std::wcout << std::endl << "Initial configuration:";
			PrintCommState(dcbSerialParams);

			std::cin.sync();	std::cin.ignore();
			while (change_params.compare("yes") && change_params.compare("no"))
				{
				std::cout << std::endl << "Change port configuration? (yes/no)" << std::endl;	std::getline(std::cin, change_params);
				}

			if (!change_params.compare("yes"))
				{
				SetCommParameters(dcbSerialParams);
				//std::cin.sync();	std::cin.ignore();

				std::wcout << std::endl << "Changing " << pcCommPort << " configuration...";
				if (!SetCommState(hSerial, &dcbSerialParams))
					{
					std::wcerr << "\aSetCommState failed with error " << GetLastError() << std::endl;
					port_dissapeared = true;
					}

				if (!port_dissapeared)
					{
					std::wcout << "SetCommState exited OK!" << std::endl;

					//  Get the comm config again.
					if (!GetCommState(hSerial, &dcbSerialParams))
						{
						std::wcerr << "\aGetCommState failed with error" << GetLastError() << std::endl;
						port_dissapeared = true;
						}

					if (!port_dissapeared)
						{
						std::wcout << std::endl << "Applied configuration:";
						PrintCommState(dcbSerialParams);
						std::wcout << std::endl << "Serial port " << pcCommPort << " successfully reconfigured!\n" << std::endl;
						}
					}
				}
			}
		// Set COM port timeout settings
		timeouts.ReadIntervalTimeout = 1; //50
		timeouts.ReadTotalTimeoutConstant = 1; //50
		timeouts.ReadTotalTimeoutMultiplier = 1; //10
		timeouts.WriteTotalTimeoutConstant = 1; //50
		timeouts.WriteTotalTimeoutMultiplier = 1; //10

		if (!port_dissapeared)
			{
			std::wcout << "Setting " << pcCommPort << " timeout settings...";
			if (!SetCommTimeouts(hSerial, &timeouts))
				{
				std::wcerr << "\aSetCommTimeouts failed with error " << GetLastError() << std::endl;
				port_dissapeared = true;
				}
			std::wcout << "SetCommTimeouts exited OK!" << std::endl;
			}

		// Send specified text (remaining command line arguments)
		if (!port_dissapeared)
			{
			std::string send_again;
			do
				{
				interrupted = false;
				sync_event = false;
				sync_complete = false;
				bool data_loss = false, waiting_read = false, result_write = false;
				DWORD bytes_written = 0, total_bytes_written = 0, bytes_read = 0, total_bytes_read = 0, dwRes = 0, drRes = 0, lastcbInQue = 0;
				std::string from_file, bytes_to_send_str;

				// Create an event object for use by WaitCommEvent. 
				if (!port_dissapeared)
					{
					oR.hEvent = CreateEvent(
						NULL,   // default security attributes 
						TRUE,   // manual-reset event 
						FALSE,  // not signaled 
						NULL    // no name
					);
					oW.hEvent = CreateEvent(
						NULL,   // default security attributes 
						TRUE,   // manual-reset event 
						FALSE,  // not signaled 
						NULL    // no name
					);

					if ((oR.hEvent == NULL) || (oW.hEvent == NULL))
						{
						std::wcerr << "\aCreateEvent failed with error " << GetLastError() << std::endl;
						port_dissapeared = true;
						}

					// Initialize the rest of the OVERLAPPED structure to zero.
					oR.Internal = 0;
					oR.InternalHigh = 0;
					oR.Offset = 0;
					oR.OffsetHigh = 0;
					//assert(oR.hEvent);
					oW.Internal = 0;
					oW.InternalHigh = 0;
					oW.Offset = 0;
					oW.OffsetHigh = 0;
					//assert(oW.hEvent);
					}

				if (send_again.compare("yes") && !change_params.compare("yes"))
					{
					std::cin.sync();	std::cin.ignore();
					}
				while (from_file.compare("yes") && from_file.compare("no"))
					{
					std::wcout << std::endl << "Read input text from file? (yes/no)" << std::endl;	std::getline(std::cin, from_file);
					}

				send_again = "0";
				if (!from_file.compare("yes"))
					{
					std::wcout << std::endl << "File name: ";
					std::string input_file_name;
					//std::cin.sync();  std::cin.ignore();
					std::getline(std::cin, input_file_name);
					std::ifstream input_file(input_file_name, std::fstream::in);
					std::string str_file((std::istreambuf_iterator<char>(input_file)), (std::istreambuf_iterator<char>()));
					bytes_to_send_str = str_file;
					input_file.close();
					}
				else
					{
					std::wcout << std::endl << "Type the string to send: ";
					//std::cin.sync();  std::cin.ignore();
					std::getline(std::cin, bytes_to_send_str);
					}

				size_t repeat_counter = 0, repetitions = 1, cycle_counter = 0, read_index = 0, read_chars = 0, write_index = 0;
				bool bad_string = false, infinite_loop = false, end_inf_cycle = false;
				do
					{
					bad_string = false;
					std::wcout << std::endl << "How many times to send? (type \"inf\" for infinite loop)" << std::endl;
					std::string repetitions_str;	std::cin >> repetitions_str;
					if (!repetitions_str.compare("inf"))
						infinite_loop = true;
					else
						{
						try
							{
							repetitions = std::stoi(repetitions_str);
							}
						catch (...)
							{
							bad_string = true;
							}
						}
					} while (bad_string);

#ifdef LOG_RECEIVED_MESSAGE
					std::ofstream flog("received_data.txt", std::fstream::app);
#endif

					const char *bytes_to_send = bytes_to_send_str.c_str();
					size_t char_count = bytes_to_send_str.length();
					char *bytes_to_read = nullptr;
					try
						{
						bytes_to_read = new char[char_count + 1];
						for (size_t i = 0; i < char_count + 1; ++i)
							{
							bytes_to_read[i] = 0;
							}
						}
					catch (const std::bad_alloc&)
						{
						throw ENoMemory;
						}					

					_COMSTAT serial_status = { 0, };
					//DWORD e = CE_BREAK | CE_OVERRUN | CE_FRAME | CE_RXOVER | CE_RXPARITY;
					DWORD e = 0;
					DWORD dFlags = PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR;

					pSData = (PSENDDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SENDDATA));
					pSData->hSerial = hSerial;
					pSData->bytes_to_Send = bytes_to_send;
					pSData->bytes_written = bytes_written;
					pSData->char_count = char_count;
					pSData->index = write_index;
					pSData->oW = oW;
					pSData->port_dissapeared = port_dissapeared;
					pSData->purgeFlags = dFlags;
					pSData->total_bytes_written = total_bytes_written;
					pSData->result_write = result_write;

					hWriteThread = (HANDLE)_beginthreadex(0, 0, &Sendthread, pSData, 0, 0);

					if (infinite_loop)
						{
						while (!(_kbhit() && _getch() == 27))
							{
							bytes_to_send = pSData->bytes_to_Send;
							bytes_written = pSData->bytes_written;
							write_index = pSData->index;
							port_dissapeared = pSData->port_dissapeared;
							total_bytes_written = pSData->total_bytes_written;
							result_write = pSData->result_write;
#ifdef ALL_CONSOLE_OUTPUT
							std::wcout << "Receiving bytes...";
#endif

							if (!ClearCommError(hSerial, &e, &serial_status))
								{
								std::wcerr << "\aClearCommError failed with error " << GetLastError() << std::endl;
								port_dissapeared = true;
								PurgeComm(hSerial, dFlags);
								break;
								}
							if (e == CE_RXOVER)
								{
								data_loss = true;
								std::wcerr << "Input RXbuffer overflow, possible loss of data!" << std::endl;
								}

							if (interrupted && (lastcbInQue == 0) && (serial_status.cbInQue > 0))
								{
								interrupted = false;
								sync_event = true;
								}
							if (sync_complete && !sync_event)
								{
								read_index = 0;
								for (size_t i = 0; i < char_count + 1; ++i)
									{
									bytes_to_read[i] = 0;
									}
								PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_RXABORT);
								sync_complete = false;
								}

							if ((serial_status.cbInQue > 0) && !sync_event && !sync_complete)
								{
								if (!waiting_read)
									{
									if (!ReadFile(hSerial, &bytes_to_read[read_index], 1, &bytes_read, &oR))
										{
										if (GetLastError() != ERROR_IO_PENDING)
											{
											std::wcerr << "\aReadFile failed with error " << GetLastError() << std::endl;
											port_dissapeared = true;
											//break;
											}
										else
											{
											waiting_read = true;
											}
										}
									else
										{
										if (bytes_read != 1)
											std::wcerr << "Bad read!" << std::endl;
										//if(bytes_to_send[read_index] != bytes_to_read[read_index])
											//std::wcerr << "Bad received " << read_index << "th character!" << std::endl;

										total_bytes_read += bytes_read;
										if (++read_index > char_count - 1)
											{
											read_index = 0;
											for (size_t i = 0; i < char_count + 1; ++i)
												{
												bytes_to_read[i] = 0;
												}
											}
										}
									}
								else
									{
									drRes = WaitForSingleObject(oR.hEvent, READ_TIMEOUT);
									switch (drRes)
										{
										case WAIT_OBJECT_0:
											if (!GetOverlappedResult(hSerial, &oR, &bytes_read, FALSE))
												{
												std::wcerr << "\aGetOverlappedResult failed with error " << GetLastError() << std::endl;
												port_dissapeared = true;
												//break;
												}
											else
												{
												if (bytes_read != 1)
													std::wcerr << "Bad read!" << std::endl;
												//if(bytes_to_send[read_index] != bytes_to_read[read_index])
													//std::wcerr << "Bad received " << read_index << "th character!" << std::endl;

												total_bytes_read += bytes_read;
												if (++read_index > char_count - 1)
													{
													read_index = 0;
													for (size_t i = 0; i < char_count + 1; ++i)
														{
														bytes_to_read[i] = 0;
														}
													}
												}
											waiting_read = false;
											break;

										case WAIT_TIMEOUT:
											if (!timer_start)
												{
												timer_start = true;
												start = std::chrono::high_resolution_clock::now();
												}
											break;

										default:
											std::wcerr << "\aWaitForSingleObject failed with error " << GetLastError() << std::endl;
											port_dissapeared = true;
											//break;
										}
									}
								}
							if (!timer_start && (serial_status.cbInQue <= lastcbInQue) && !sync_event)
								{
								timer_start = true;
								start = std::chrono::high_resolution_clock::now();
								}
							else if ((serial_status.cbInQue > lastcbInQue) || sync_event)
								{
								timer_start = false;
								elapsed_time = 0;
								}
							lastcbInQue = serial_status.cbInQue;

							if (timer_start)
								{
								stop = std::chrono::high_resolution_clock::now();
								elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
								}

							if (elapsed_time > DETECTION_TIMEOUT)
								{
								interrupted = true;
								std::wcerr << "Communication interrupted!" << std::endl;
								}

							


#ifdef ALL_CONSOLE_OUTPUT
							std::wcout << bytes_read << " bytes read\n\n";
#endif


#ifdef ALL_CONSOLE_OUTPUT
							std::wcout << "Received message:\n\n" << bytes_to_read << "\n" << std::endl;
#endif
#ifdef LOG_RECEIVED_MESSAGE
							flog << repeat_counter + 1 << ".: " << bytes_to_read << std::endl;
#endif

							if (!total_bytes_written && result_write)
								std::wcerr << "0 total bytes have been sent!" << std::endl;
							//if (strncmp(bytes_to_send, bytes_to_read, read_index) && !waiting_read)
								//std::wcerr << "Bad received string!\n" << std::endl;

							++cycle_counter;
#ifdef LOG_RECEIVED_MESSAGE
							flog << repeat_counter << ".: " << bytes_to_read << std::endl;
#endif

							if (port_dissapeared)
								{
								PurgeComm(hSerial, dFlags);
								break;
								}
							}

						std::wcerr << "\ttotal " << total_bytes_written << " bytes written" << std::endl;
						std::wcerr << "\ttotal " << total_bytes_read << " bytes read" << std::endl;
						std::wcerr << "\tString length was: " << bytes_to_send_str.length() << "\n" << std::endl;

						if (data_loss)
							std::wcerr << "Some data may have been lost due to RXbuffer overflow!" << std::endl;
						}
					else
						{
						while (repeat_counter < repetitions)
							{
							for (size_t i = 0; i < char_count + 1; ++i)
								{
								bytes_to_read[i] = 0;
								}

#ifdef ALL_CONSOLE_OUTPUT
							std::wcout << std::endl << "Sending bytes...";
#endif
							size_t current_i = 0;
							for (size_t i = 0; i < bytes_to_send_str.length(); ++i)
								{
								if (!WriteFile(hSerial, &bytes_to_send[i], 1, &bytes_written, NULL))
									{
									std::wcerr << "\aWriteFile failed with error " << GetLastError() << std::endl;
									port_dissapeared = true;
									break;
									}
#ifdef ALL_CONSOLE_OUTPUT
								std::wcout << bytes_written << " bytes written" << std::endl;
#endif

#ifdef ALL_CONSOLE_OUTPUT
								std::wcout << "Receiving bytes...";
#endif
								if (!ReadFile(hSerial, &bytes_to_read[i], 1, &bytes_read, NULL))
									{
									std::wcerr << "\aReadFile failed with error " << GetLastError() << std::endl;
									port_dissapeared = true;
									break;
									}
#ifdef ALL_CONSOLE_OUTPUT
								std::wcout << bytes_read << " bytes read\n\n";
#endif
								total_bytes_written += bytes_written;
								total_bytes_read += bytes_read;

								if (!bytes_written)
									std::wcerr << "0 bytes have been sent!" << std::endl;
								if (bytes_read != bytes_written)
									std::wcerr << "Communication interrupted!" << std::endl;
								if (bytes_to_send[i] != bytes_to_read[i])
									std::wcerr << "Bad received " << i << "th character!" << std::endl;

#ifdef ALL_CONSOLE_OUTPUT
								std::wcout << "Received message:\n\n" << bytes_to_read << "\n" << std::endl;
#endif
								current_i = i;
								if (_kbhit() && _getch() == 27)
									{
									end_inf_cycle = true; break;
									}
								}

							if (!total_bytes_written)
								std::wcerr << "0 total bytes have been sent!" << std::endl;
							if (strncmp(bytes_to_send, bytes_to_read, current_i + 1))
								std::wcerr << "Bad received string!\n" << std::endl;

							++repeat_counter;

#ifdef LOG_RECEIVED_MESSAGE
							flog << repeat_counter << ".: " << bytes_to_read << std::endl;
#endif

							if (port_dissapeared || end_inf_cycle)
								break;
							}

						std::wcerr << "\ttotal " << total_bytes_written << " bytes written" << std::endl;
						std::wcerr << "\ttotal " << total_bytes_read << " bytes read" << std::endl;
						std::wcerr << "\tString length was: " << bytes_to_send_str.length() << "\n" << std::endl;
						}

						delete[] bytes_to_read;
						end_thread = true;
						WaitForSingleObject(hWriteThread, INFINITE);
						CloseHandle(hWriteThread);
						CloseHandle(oW.hEvent);
						CloseHandle(oR.hEvent);
						HeapFree(GetProcessHeap(), 0, pSData);

					if (port_dissapeared)
						send_again = "no";
					else
						{
						std::cin.sync();  std::cin.ignore();
						while (send_again.compare("yes") && send_again.compare("no"))
							{
							std::cout << std::endl << "Send another string? (yes/no)" << std::endl;	std::getline(std::cin, send_again);
							}
						}


					
#ifdef LOG_RECEIVED_MESSAGE
					flog.close();
#endif
				} while (!send_again.compare("yes"));
			}

		if (port_noexist || port_dissapeared)
			{
			std::cin.sync();  std::cin.ignore();
			}
		while (new_port.compare("yes") && new_port.compare("no"))
			{
			std::cout << std::endl << "Open a new port for communication? (yes/no)" << std::endl;	std::getline(std::cin, new_port);
			}

		// Close serial port
		if (!port_noexist)
			{
			std::wcout << std::endl << "Closing serial port " << pcCommPort << "...";
			CloseHandle(hSerial);
			std::wcout << "Serial port closed!" << std::endl;
			}

		} while (!new_port.compare("yes"));

		//DeleteFile(hSerial);
		std::cout << "\nPress Enter to exit." << std::endl;
		std::cin.ignore();
		// exit normally
		return 0;
	}

	unsigned int __stdcall Sendthread(void* aSData)
		{
		PSENDDATA pSData = (PSENDDATA)aSData;
		SENDDATA SData = *pSData;
		DWORD dwRes = 0;

		while (!end_thread)
			{
			if (sync_event)
				{
				sync_event = false;
				sync_complete = true;
				PurgeComm(SData.hSerial, PURGE_TXCLEAR | PURGE_TXABORT);
				SData.index = 0;
				}

			if (!WriteFile(SData.hSerial, &SData.bytes_to_Send[SData.index], 1, &SData.bytes_written, &SData.oW))
				{
				if (GetLastError() != ERROR_IO_PENDING)
					{
					std::wcerr << "\aWriteFile failed with error " << GetLastError() << std::endl;
					SData.port_dissapeared = true;
					SData.result_write = false;
					//break;
					}
				else
					{
					dwRes = WaitForSingleObject(SData.oW.hEvent, WRITE_TIMEOUT);
					switch (dwRes)
						{
						case WAIT_OBJECT_0:
							if (!GetOverlappedResult(SData.hSerial, &SData.oW, &SData.bytes_written, FALSE))
								SData.result_write = false;
							else
								{
								SData.result_write = true;
								SData.total_bytes_written += SData.bytes_written;
								if (!SData.bytes_written)
									std::wcerr << "0 bytes have been sent!" << std::endl;
								if (++SData.index > SData.char_count - 1)
									SData.index = 0;
								}
							break;

						case WAIT_TIMEOUT:
							break;

						default:
							std::wcerr << "\aWaitForSingleObject failed with error " << GetLastError() << std::endl;
							SData.port_dissapeared = true;
							SData.result_write = false;
							//break;
						}
					}
				}
			else
				{
				SData.result_write = true;
				SData.total_bytes_written += SData.bytes_written;
				if (!SData.bytes_written)
					std::wcerr << "0 bytes have been sent!" << std::endl;
				if (++SData.index > SData.char_count - 1)
					SData.index = 0;
				}

			*pSData = SData;

			if (SData.port_dissapeared)
				{
				PurgeComm(SData.hSerial, SData.purgeFlags);
				break;
				}
			}

		return 0;
		}