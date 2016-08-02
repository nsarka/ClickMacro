/*
	ClickMacro. Written July 29, 2016 to August 1, 2016.
	Instructions written in the program.
*/

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <Windows.h>

enum STATES
{
	IDLE = 0,
	RECORD,
	PLAY,
	CLEAR
};

struct Step
{
	POINT pos;
	bool leftclick = true;
	int wait_time;
};

static int state = 0;
static bool loop = false;

POINT GetCursorPoint()
{
	POINT pos;
	GetCursorPos(&pos);
	return pos;
}

void UserInput()
{
	std::cout << "ClickMacro.\nEmail: nsarka00@gmail.com\n" << std::endl;
	std::cout << "* R (F1) - record\n* P - play\n* L - loop play\n* C - clear\n* any other key (F2) - idle/stop" << std::endl;
	std::cout << "\nRecording over an existing script will add clicks to the end of the list.\n\n" << std::endl;

	while (1)
	{
		char choice;
		std::cin >> choice;
		switch (choice)
		{
			case 'r':
			{
				state = RECORD;
				std::cout << "Changed state to record." << std::endl;
				break;
			}
			case 'p':
			{
				state = PLAY;
				loop = false;
				std::cout << "Changed state to play." << std::endl;
				break;
			}
			case 'c':
			{
				state = CLEAR;
				std::cout << "Clearing..." << std::endl;
				break;
			}
			case 'l':
			{
				state = PLAY;
				loop = true;
				std::cout << "Changed state to play (loop)." << std::endl;
				break;
			}
			default:
			{
				state = IDLE;
				loop = false;
				std::cout << "Changed state to idle (default)." << std::endl;
				break;
			}
		}
	}
}

void UserKeyB()
{
	while(1)
	{
		if ((GetKeyState(VK_F1) & 0x8000) && state != RECORD)
		{
			state = RECORD;
			std::cout << "Changed state to record." << std::endl;
		}
		else if ((GetKeyState(VK_F2) & 0x8000) && state != IDLE)
		{
			state = IDLE;
			std::cout << "Changed state to idle (default)." << std::endl;
			loop = false;
		}
	}
}

int main()
{
	std::thread t(UserInput);
	std::thread t2(UserKeyB);

	std::vector<Step> steps;
	std::chrono::high_resolution_clock::time_point start_time, end_time;
	std::chrono::duration<long long, std::nano> difference;
	bool mousedown = false;
	bool timing = false;

	while (1)
	{
		switch (state)
		{
			case IDLE:
			{
				if(timing)
					timing = false;
				break;
			}
			case RECORD:
			{
				if ((GetKeyState(VK_LBUTTON) & 0x8000) || (GetKeyState(VK_RBUTTON) & 0x8000))
				{
					if (!mousedown)
					{
						if (timing)
						{
							end_time = std::chrono::high_resolution_clock::now();
							difference = end_time - start_time;

							Step step;
							step.pos = GetCursorPoint();
							step.wait_time = std::chrono::duration_cast<std::chrono::milliseconds>(difference).count();
							if ((GetKeyState(VK_RBUTTON) & 0x8000))
								step.leftclick = false;
							steps.push_back(step);

							std::cout << "Recorded click: " << step.wait_time << " ms." << std::endl;

							mousedown = true;
							timing = false;
						}
					}
				}
				else if (!(GetKeyState(VK_LBUTTON) & 0x8000) && !(GetKeyState(VK_RBUTTON) & 0x8000))
				{
					mousedown = false;
					if (!timing)
					{
						start_time = std::chrono::high_resolution_clock::now();
						timing = true;
					}
				}
				break;
			}
			case PLAY:
			{
				timing = false;

				do
				{
					for (Step i : steps)
					{
						if (state != PLAY)	//In case the user switches state mid-loop
						{
							loop = false;
							break;
						}

						INPUT input = { 0 };
						input.type = INPUT_MOUSE;
						input.mi.mouseData = 0;
						input.mi.dx = i.pos.x*(65536.0f / GetSystemMetrics(SM_CXSCREEN));
						input.mi.dy = i.pos.y*(65536.0f / GetSystemMetrics(SM_CYSCREEN));

						if (i.leftclick)
						{
							input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP);
						}
						else
						{
							input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP);
						}

						SendInput(1, &input, sizeof(input));
						std::cout << "Sleeping for: " << i.wait_time << " ms." << std::endl;
						Sleep(i.wait_time);
					}
				} while (loop);

				state = IDLE;
				std::cout << "Changed state to idle (default)." << std::endl;
				break;
			}
			case CLEAR:
			{
				timing = false;
				steps.clear();
				std::cout << "Cleared." << std::endl;
				state = IDLE;
				std::cout << "Changed state to idle (default)." << std::endl;
				break;
			}
		}
	}
	
	return 0;
}
