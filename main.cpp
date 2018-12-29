// Include
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include <conio.h>
#include <ctype.h>
#include <math.h>

#include "renderSystem.h"
#include "input.h"
#include "level.h"
#include "unitData.h"

#define M_PI 3.14159265358979323846

// Constants
const int maxUnitsCount = 35;

const float cellBeginValue = 0.001f;
const float cellEndValue = 0.999f;

// Logics variables
bool isGameActive = true;
clock_t clockLastFrame = 0;

int framesCounter = 0;
float framesTimeCounter = 0;
int fps = 0;
int sinShift = 0;

unsigned char levelData[levelRows][levelColumns];

bool startMainGame = false;

UnitData unitsData[maxUnitsCount];
int unitsCount = 0;
int heroIndex = 0;

// Functions
void SystemSetup()
{
	srand(time(0));

	// Initialize render system
	RenderSystemInitialize();
}

void Initialize()
{
	// Set clockLastFrame start value
	clockLastFrame = clock();

	unitsCount = 0;

	// Load level
	for (int r = 0; r < levelRows; r++)
	{
		for (int c = 0; c < levelColumns; c++)
		{
			unsigned char cellSymbol = levelData0[r][c];

			levelData[r][c] = cellSymbol;

			switch (cellSymbol)
			{
				case CellSymbol_Hero:
					heroIndex = unitsCount;

				case CellSymbol_Goomba:
					UnitType unitType = GetUnitTypeFromCell(cellSymbol);
					unitsData[unitsCount].type = unitType;
					unitsData[unitsCount].y = float(r);
					unitsData[unitsCount].x = float(c);
					unitsData[unitsCount].health = 1;
					unitsData[unitsCount].ySpeed = 0.0;
					unitsData[unitsCount].xSpeed = 0.0;
					unitsData[unitsCount].yOrder = UnitOrder_None;
					unitsData[unitsCount].xOrder = UnitOrder_None;

					unitsCount++;
			}
		}
	}
}

void Render()
{
	// Start frame
	RenderSystemClear();

	// Draw frame (Test)
	if (!startMainGame)
	{
		RenderSystemDrawText(8, 28, "Press any arrow", ConsoleColor_Green, ConsoleColor_Black);

		if (IsKeyDown(VK_UP))
		{
			RenderSystemDrawChar(10, 35, 24, ConsoleColor_Red, ConsoleColor_Gray);
			sinShift++;
		}

		if (IsKeyDown(VK_DOWN))
		{
			RenderSystemDrawChar(12, 35, 25, ConsoleColor_Blue, ConsoleColor_White);
			sinShift++;
		}

		if (IsKeyDown(VK_RIGHT))
		{
			RenderSystemDrawChar(12, 37, 26, ConsoleColor_Yellow, ConsoleColor_DarkGray);
			sinShift++;
		}

		if (IsKeyDown(VK_LEFT))
		{
			RenderSystemDrawChar(12, 33, 27, ConsoleColor_Green, ConsoleColor_DarkCyan);
			sinShift++;
		}
	}

	// Draw sine wave
	if (!startMainGame)
	{
		int yStartPoint = 17;

		int rateSlower = 1500;
		if (fps != 0)
		{
			int rateSlower = fps;
		}

		for (int i = 0; i < screenColumns; i++)
		{
			//
			int pointX = i;
			int pointY = yStartPoint + round((sin((i + (sinShift / rateSlower)) * M_PI / 8) * 3));
			RenderSystemDrawChar(pointY, pointX, 0, ConsoleColor_Red, ConsoleColor_Red);
		}
	}

	// Draw level
	if (IsKeyDown(VK_RETURN))
	{
		startMainGame = true;
	}

	if (startMainGame)
	{
		for (int r = 0; r < levelRows; r++)
		{
			for (int c = 0; c < levelColumns; c++)
			{
				unsigned char cellSymbol = levelData[r][c];

				unsigned char renderSymbol = GetRenderCellSymbol(cellSymbol);
				ConsoleColor symbolColor = GetRenderCellSymbolColor(cellSymbol);
				ConsoleColor backgroundColor = GetRenderCellSymbolBackgroundColor(cellSymbol);

				if (cellSymbol == CellSymbol_Hero)
				{
					symbolColor = GetRenderHeroColor(unitsData[heroIndex].health);
				}

				RenderSystemDrawChar(r, c, renderSymbol, symbolColor, backgroundColor);
			}
		}
	}
	else
	{
		RenderSystemDrawText(22, 22, "Press Enter when you're ready", ConsoleColor_Green, ConsoleColor_Black);
	}

	// Draw FPS
	char textBuffer[32];
	sprintf_s(textBuffer, "FPS: %d", fps);
	RenderSystemDrawText(0, 0, textBuffer, ConsoleColor_DarkGreen, ConsoleColor_Black);

	// End frame
	RenderSystemFlush();
}

UnitData* GetUnitAt(int row, int column)
{
	for (int u = 0; u  < unitsCount; u++)
	{
		if (unitsData[u].health <= 0)
		{
			continue;
		}
		if (int(unitsData[u].y) == row && int(unitsData[u].x) == column)
		{
			return &unitsData[u];
		}
	}
}

void KillUnit(UnitData* pointerToUnitData)
{
	pointerToUnitData->health = 0;
	int row = int(pointerToUnitData->x);
	int column = int(pointerToUnitData->y);
	levelData[row][column] = CellSymbol_Empty;
}

void UpdateAI()
{
	for (int u = 0; u < unitsCount; u++)
	{
		// Ignore Hero
		if (unitsData[u].type == UnitType_Hero)
		{
			continue;
		}

		// Ignore dead units
		if (unitsData[u].health <= 0)
		{
			continue;
		}

		int row = int(unitsData[u].x);
		int column = int(unitsData[u].y);

		if (unitsData[u].xOrder == UnitOrder_None)
		{
			// Start move to empty cell
			if (levelData[row][column - 1] == CellSymbol_Empty)
			{
				unitsData[u].xOrder = UnitOrder_Backward;
			}
			else
			{
				unitsData[u].xOrder = UnitOrder_Forward;
			}
		}
		else
		{
			if (unitsData[u].xOrder == UnitOrder_Backward)
			{
				unsigned char cellLeft = levelData[row][column - 1];
				UnitType unitType = GetUnitTypeFromCell(cellLeft);

				// Can not move cell left
				if ((unitsData[u].x <= (column + cellBeginValue)) && cellLeft != CellSymbol_Empty && unitType != UnitType_None)
				{
					unitsData[u].xOrder = UnitOrder_Forward;
				}
			}
			else
			{
				unsigned char cellRight = levelData[row][column + 1];
				UnitType unitType = GetUnitTypeFromCell(cellRight);

				// Can not move cell right
				if ((unitsData[u].x >= (column + cellEndValue)) && cellRight != CellSymbol_Empty && unitType != UnitType_None)
				{
					unitsData[u].xOrder = UnitOrder_Backward;
				}
				
			}
		}
	}
}

bool MoveUnitTo(UnitData* pointerToUnitData, float newX, float newY)
{
	// Ignore dead units
	if (pointerToUnitData->health <= 0)
	{
		return false;
	}
	bool canMoveToCell = false;

	int newRow = int(newY);
	int newColumn = int(newX);
	int oldRow = int(pointerToUnitData->y);
	int oldColumn = int(pointerToUnitData->x);

	unsigned char unitSymbol = levelData[oldRow][oldColumn];
	unsigned char destinationCellSymbol = levelData[newRow][newColumn];

	int directionRow = newRow - oldRow;
	int directionColumn = newColumn - oldColumn;

	// All units actions
	switch (destinationCellSymbol)
	{
	case CellSymbol_Empty:
		canMoveToCell = true;
		break;

	case CellSymbol_Abyss:
		KillUnit(pointerToUnitData);
		break;

	case CellSymbol_Box:
		if (directionRow < 0)
		{
			levelData[newRow - 1][newColumn] = CellSymbol_Crystal;
			levelData[newRow][newColumn] = CellSymbol_OpenedBox;
		}
		break;

	case CellSymbol_MushroomBox:
		if (directionRow < 0)
		{
			levelData[newRow - 1][newColumn] = CellSymbol_Mushroom;
			levelData[newRow][newColumn] = CellSymbol_OpenedBox;
		}
		break;

	}

	// Only hero actions
	if (pointerToUnitData->type == UnitType_Hero)
	{
		switch (destinationCellSymbol)
		{
		case CellSymbol_Exit:
			isGameActive = false;
			break;

		case CellSymbol_Crystal:
			canMoveToCell = true;
			break;

		case CellSymbol_Mushroom:
			canMoveToCell = true;
			if (pointerToUnitData->health < 2)
			{
				pointerToUnitData->health = 2;
			}
			break;

		case CellSymbol_BrickWall:
			if ((directionRow < 0) && (pointerToUnitData->health > 1))
			{
				levelData[newRow][newColumn] = CellSymbol_Empty;
			}
			break;

		case CellSymbol_Goomba:
			if (directionRow > 0)
			{
				UnitData* unitData = GetUnitAt(newRow, newColumn);
				if (unitData != 0)
				{
					KillUnit(unitData);
					pointerToUnitData->ySpeed = -GetUnitJumpSpeed(pointerToUnitData->type);
				}
			}
			break;
		}
	}
	// Only monster actions
	else
	{
		switch (destinationCellSymbol)
		{
		case CellSymbol_Hero:
			unitsData[heroIndex].health--;

			if (pointerToUnitData->xOrder == UnitOrder_Backward)
			{
				pointerToUnitData->xOrder = UnitOrder_Forward;
			}
			else
			{
				pointerToUnitData->xOrder = UnitOrder_Backward;
			}
			break;

		default:
			UnitType unitType = GetUnitTypeFromCell(destinationCellSymbol);
			if (unitType != UnitType_None)
			{
				if (pointerToUnitData->xOrder == UnitOrder_Backward)
				{
					pointerToUnitData->xOrder = UnitOrder_Forward;
				}
				else
				{
					pointerToUnitData->xOrder = UnitOrder_Backward;
				}
			}
			break;
		}
	}

	if (canMoveToCell)
	{
		levelData[oldRow][oldColumn] = CellSymbol_Empty;

		pointerToUnitData->x = newX;
		pointerToUnitData->y = newY;

		levelData[newRow][newColumn] = unitSymbol;
	}

	return canMoveToCell;
}

void UpdateUnit(UnitData* pointerToUnitData, float deltaTime)
{
	// Unit row and column
	int row = int(pointerToUnitData->y);
	int column = int(pointerToUnitData->x);

	// Y order
	if (pointerToUnitData->yOrder == UnitOrder_Backward)
	{
		// Jump
		if (   (pointerToUnitData->y >= (row + cellEndValue))
			&& (levelData[row + 1][column] != CellSymbol_Empty)
			&& (levelData[row + 1][column] != CellSymbol_Abyss))
		{
			pointerToUnitData->ySpeed = -GetUnitJumpSpeed(pointerToUnitData->type);
		}
	}

	// X order
	if (pointerToUnitData->xOrder == UnitOrder_Backward)
	{
		pointerToUnitData->xSpeed = -GetUnitSpeedX(pointerToUnitData->type);
	}
	else
	{
		if (pointerToUnitData->xOrder == UnitOrder_Forward)
		{
			pointerToUnitData->xSpeed = GetUnitSpeedX(pointerToUnitData->type);
		}
		else
		{
			pointerToUnitData->xSpeed = 0;
		}
	}
	
	// New position
	float deltaY = pointerToUnitData->ySpeed * deltaTime;
	float deltaX = pointerToUnitData->xSpeed * deltaTime;
	float newY = pointerToUnitData->y + deltaY;
	float newX = pointerToUnitData->x + deltaX;
	int newRow = int(newY);
	int newColumn = int(newX);

	// Y(row) step
	if (newRow != row)
	{
		// If unit can go to cell
		if (MoveUnitTo(pointerToUnitData, pointerToUnitData->x, newY))
		{
			row = int(pointerToUnitData->y);
		}
		else
		{
			// Can not move cell down
			if (newRow > row)
			{
				pointerToUnitData->y = row + cellEndValue;

				if (pointerToUnitData->ySpeed > 0.0)
				{
					pointerToUnitData->ySpeed = 0.0;
				}
			}
			// Can not move cell up
			else
			{
				pointerToUnitData->y = row + cellBeginValue;
				
				if (pointerToUnitData->ySpeed < 0.0)
				{
					pointerToUnitData->ySpeed = 0.0;
				}
			}
		}
	}
	else
	{
		pointerToUnitData->y = newY;
	}

	// X(column) step
	if (newColumn != column)
	{
		// If unit can go to cell
		if (MoveUnitTo(pointerToUnitData, newX, pointerToUnitData->y))
		{
			column = int(pointerToUnitData->x);
		}
		else
		{
			// Can not move cell right
			if (newColumn > column)
			{
				pointerToUnitData->x = column + cellEndValue;

				if (pointerToUnitData->xSpeed > 0.0)
				{
					pointerToUnitData->xSpeed = 0.0;
				}
			}
			// Can not move cell left
			else
			{
				pointerToUnitData->x = column + cellBeginValue;

				if (pointerToUnitData->xSpeed < 0.0)
				{
					pointerToUnitData->xSpeed = 0.0;
				}
			}
		}
	}
	else
	{
		pointerToUnitData->x = newX;
	}

	// Gravity
	pointerToUnitData->ySpeed += gravitySpeed * deltaTime;
	if (pointerToUnitData->ySpeed > gravitySpeed)
	{
		pointerToUnitData->ySpeed = gravitySpeed;
	}
}

void Update()
{
	// Calculate delta time
	clock_t clockNow = clock();
	clock_t deltaClock = clockNow - clockLastFrame;
	float deltaTime = float(deltaClock) / CLOCKS_PER_SEC;
	clockLastFrame = clockNow;

	// Calculate FPS
	framesCounter++;
	framesTimeCounter += deltaTime;
	if (framesTimeCounter >= 1.0)
	{
		framesTimeCounter -= 1.0;
		fps = framesCounter;
		framesCounter = 0;
	}

	if (startMainGame)
	{
		// Hero control
		// W key (up)
		if (IsKeyDown(0x57))
		{
			unitsData[heroIndex].yOrder = UnitOrder_Backward;
		}
		else
		{
			unitsData[heroIndex].yOrder = UnitOrder_None;
		}

		// A key(left)
		if (IsKeyDown(0x41))
		{
			unitsData[heroIndex].xOrder = UnitOrder_Backward;
		}
		else
		{
			// D key (right)
			if (IsKeyDown(0x44))
			{
				unitsData[heroIndex].xOrder = UnitOrder_Forward;
			}
			else
			{
				unitsData[heroIndex].xOrder = UnitOrder_None;
			}
		}

		// Update all units
		for (int u = 0; u < unitsCount; u++)
		{
			UpdateUnit(&unitsData[u], deltaTime);
		}

		// Update AI
		UpdateAI();

		// Hero dead
		if (unitsData[heroIndex].health <= 0)
		{
			Initialize();
		}
	}
}

void Shutdown()
{

}

int main()
{
	SystemSetup();
	Initialize();

	while (isGameActive)
	{
		Render();
		Update();
	}
	
	Shutdown();
	return 0;
}