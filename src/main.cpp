#include "serial_test.h"


int _tmain(int argc, TCHAR *argv[])
	{
	// Declare variables and structures
	HANDLE hSerial;
	DCB dcbSerialParams = { 0, };
	COMMTIMEOUTS timeouts = { 0, };
	//OVERLAPPED o;	
	bool port_dissapeared = false, port_noexist = false;
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
		hSerial = CreateFile(pcCommPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL /*| FILE_FLAG_OVERLAPPED*/, NULL);

		if(hSerial == INVALID_HANDLE_VALUE)
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
*//*
		// Create an event object for use by WaitCommEvent. 
		o.hEvent = CreateEvent(
			NULL,   // default security attributes 
			TRUE,   // manual-reset event 
			FALSE,  // not signaled 
			NULL    // no name
		);

		// Initialize the rest of the OVERLAPPED structure to zero.
		o.Internal = 0;
		o.InternalHigh = 0;
		o.Offset = 0;
		o.OffsetHigh = 0;
		assert(o.hEvent);
*/
		//  Initialize the DCB structure.
		SecureZeroMemory(&dcbSerialParams, sizeof(DCB));
		dcbSerialParams.DCBlength = sizeof(DCB);

		//  Build on the current configuration by first retrieving all current settings.
		if(!port_dissapeared && !GetCommState(hSerial, &dcbSerialParams))
			{
			std::wcerr << "\aGetCommState failed with error " << GetLastError() << std::endl;
			port_dissapeared = true;
			}

		std::string change_params("0");
		if(!port_dissapeared)
			{
			std::wcout << std::endl << "Initial configuration:";
			PrintCommState(dcbSerialParams);

			std::cin.sync();	std::cin.ignore();
			while(change_params.compare("yes") && change_params.compare("no"))
				{	std::cout << std::endl << "Change port configuration? (yes/no)" << std::endl;	std::getline(std::cin, change_params);	}

			if(!change_params.compare("yes"))
				{
				SetCommParameters(dcbSerialParams);
				//std::cin.sync();	std::cin.ignore();

				std::wcout << std::endl << "Changing " << pcCommPort << " configuration...";
				if(!SetCommState(hSerial, &dcbSerialParams))
					{
					std::wcerr << "\aSetCommState failed with error " << GetLastError() << std::endl;
					port_dissapeared = true;
					}

				if (!port_dissapeared)
					{
					std::wcout << "SetCommState exited OK!" << std::endl;

				//  Get the comm config again.
					if(!GetCommState(hSerial, &dcbSerialParams))
						{
						std::wcerr << "\aGetCommState failed with error" << GetLastError() << std::endl;
						port_dissapeared = true;
						}

					if(!port_dissapeared)
						{
						std::wcout << std::endl << "Applied configuration:";
						PrintCommState(dcbSerialParams);
						std::wcout << std::endl << "Serial port " << pcCommPort << " successfully reconfigured!\n" << std::endl;
						}
					}
				}
			}
		// Set COM port timeout settings
		timeouts.ReadIntervalTimeout = 50; //50
		timeouts.ReadTotalTimeoutConstant = 50; //50
		timeouts.ReadTotalTimeoutMultiplier = 10; //10
		timeouts.WriteTotalTimeoutConstant = 50; //50
		timeouts.WriteTotalTimeoutMultiplier = 10; //10

		if(!port_dissapeared)
			{
			std::wcout << "Setting " << pcCommPort << " timeout settings...";
			if(!SetCommTimeouts(hSerial, &timeouts))
				{
				std::wcerr << "\aSetCommTimeouts failed with error " << GetLastError() << std::endl;
				port_dissapeared = true;
				}
			std::wcout << "SetCommTimeouts exited OK!" << std::endl;
			}

		// Send specified text (remaining command line arguments)
		if(!port_dissapeared)
			{
			std::string send_again;
			do
				{
				DWORD bytes_written = 0, total_bytes_written = 0, bytes_read = 0, total_bytes_read = 0;
				std::string from_file, bytes_to_send_str;

				if(send_again.compare("yes") && !change_params.compare("yes"))
					{	std::cin.sync();	std::cin.ignore();	}
				while(from_file.compare("yes") && from_file.compare("no"))
					{	std::wcout << std::endl << "Read input text from file? (yes/no)" << std::endl;	std::getline(std::cin, from_file);	}

				send_again = "0";
				if(!from_file.compare("yes"))
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

				size_t repeat_counter = 0, repetitions = 1, cycle_counter = 0;
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
							{	repetitions = std::stoi(repetitions_str);	}
						catch (...)
							{	bad_string = true;	}
						}
					} while(bad_string);

#ifdef LOG_RECEIVED_MESSAGE
				std::ofstream flog("received_data.txt", std::fstream::app);
#endif

				/*
				std::string string_memory(bytes_to_send_str);
				for (size_t i = 1; i < repetitions; ++i)
					bytes_to_send_str += string_memory;
				*/

				const char *bytes_to_send = bytes_to_send_str.c_str();
				size_t char_count = bytes_to_send_str.length();
				char *bytes_to_read = nullptr;
				try
					{	bytes_to_read = new char[char_count + 1];	}
				catch (const std::bad_alloc&)
					{	throw ENoMemory;	}

				if (infinite_loop)
					{
					for (;;)
						{
						for (size_t i = 0; i < char_count + 1; ++i)
							{	bytes_to_read[i] = 0;	}

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
#ifdef LOG_RECEIVED_MESSAGE
							flog << repeat_counter + 1 << ".: " << bytes_to_read << std::endl;
#endif
							current_i = i;
							if (_kbhit() && _getch() == 27)
								{	end_inf_cycle = true; break;	}
							}

						if (!total_bytes_written)
							std::wcerr << "0 total bytes have been sent!" << std::endl;
						if (strncmp(bytes_to_send, bytes_to_read, current_i+1))
							std::wcerr << "Bad received string!\n" << std::endl;

						++cycle_counter;
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
				else
					{
					while (repeat_counter < repetitions)
						{
						for (size_t i = 0; i < char_count + 1; ++i)
							{	bytes_to_read[i] = 0;	}

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
								{	end_inf_cycle = true; break;	}
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

				if(port_dissapeared)
					send_again = "no";
				else
					{
					std::cin.sync();  std::cin.ignore();
					while(send_again.compare("yes") && send_again.compare("no"))
						{	std::cout << std::endl << "Send another string? (yes/no)" << std::endl;	std::getline(std::cin, send_again);	}
					}

				delete[] bytes_to_read;
#ifdef LOG_RECEIVED_MESSAGE
				flog.close();
#endif
				} while(!send_again.compare("yes"));
			}

		if(port_noexist || port_dissapeared)
			{	std::cin.sync();  std::cin.ignore();	}
		while(new_port.compare("yes") && new_port.compare("no"))
			{	std::cout << std::endl << "Open a new port for communication? (yes/no)" << std::endl;	std::getline(std::cin, new_port);	}
			
		// Close serial port
		if(!port_noexist)
			{
			std::wcout << std::endl << "Closing serial port " << pcCommPort << "...";
			CloseHandle(hSerial);
			std::wcout << "Serial port closed!" << std::endl;
			}

		} while(!new_port.compare("yes"));	

	//DeleteFile(hSerial);
	std::cout << "\nPress Enter to exit." << std::endl;
	std::cin.ignore();
	// exit normally
	return 0;
	}