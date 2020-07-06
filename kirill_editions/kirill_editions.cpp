
#include <iostream>
#include <string>
using namespace std;

#include "olcConsoleGameEngine.h"


struct sBall
{
	float px, py;
	float vx, vy;
	float ax, ay;
	float radius;
	float mass;

	int id;
};
struct sLineSegment
{
float sx, sy;
float ex, ey;
float radius;
};

class CirclePhysics : public olcConsoleGameEngine
{
public:
	CirclePhysics()
	{
		m_sAppName = L"Circle Physics";
	}

private:
	vector<sLineSegment> vecLines;
	sLineSegment* pSelectedLine = nullptr;
	bool bSelectedLineStart = false;
	vector<pair<float, float>> modelCircle;
	vector<sBall> vecBalls;
	sBall *pSelectedBall = nullptr;


	// Adds a ball to the vector
	void AddBall(float x, float y, float r = 5.0f)
	{
		sBall b;
		b.px = x; b.py = y;
		b.vx = 0; b.vy = 0;
		b.ax = 0; b.ay = 0;
		b.radius = r;
		b.mass = r * 10.0f;

		b.id = vecBalls.size();
		vecBalls.emplace_back(b);
	}


public:
	bool OnUserCreate()
	{
		// Define Circle Model
		modelCircle.push_back({ 0.0f, 0.0f });
		int nPoints = 20;
		for (int i = 0; i < nPoints; i++)
			modelCircle.push_back({ cosf(i / (float)(nPoints - 1) * 2.0f * 3.14159f) , sinf(i / (float)(nPoints - 1) * 2.0f * 3.14159f) });

		float fDefaultRad = 4.0f;
		AddBall(ScreenWidth() * 0.25f, ScreenHeight() * 0.5f, fDefaultRad);
		AddBall(ScreenWidth() * 0.75f, ScreenHeight() * 0.5f, fDefaultRad);

//add lines
float fLineRadius = 0.5f;
vecLines.push_back({ 8.0f, 68.0f, 80.0f, 4.0f, fLineRadius });//up left
vecLines.push_back({ 80.0f, 4.0f, 150.0f, 68.0f, fLineRadius });//up right

vecLines.push_back({ 8.0f, 68.0f, 150.0f, 68.0f, fLineRadius });//down
		
// Add 10 Random Balls
	/*	for (int i = 0; i < 10; i++)
			AddBall(rand() % ScreenWidth(), rand() % ScreenHeight(), rand() % 16 + 2);*/


		return true;
	}

	bool OnUserUpdate(float fElapsedTime)
	{
		auto DoCirclesOverlap = [](float x1, float y1, float r1, float x2, float y2, float r2)
		{
			return fabs((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2)) <= (r1 + r2)*(r1 + r2);
		};

		auto IsPointInCircle = [](float x1, float y1, float r1, float px, float py)
		{
			return fabs((x1 - px)*(x1 - px) + (y1 - py)*(y1 - py)) < (r1 * r1);
		};

		if (m_mouse[0].bPressed || m_mouse[1].bPressed)
		{
			pSelectedBall = nullptr;
			for (auto &ball : vecBalls)
			{
				if (IsPointInCircle(ball.px, ball.py, ball.radius, m_mousePosX, m_mousePosY))
				{
					pSelectedBall = &ball;
					break;
				}
			}
		}

		if (m_mouse[0].bHeld)
		{
			if (pSelectedBall != nullptr)
			{
				pSelectedBall->px = m_mousePosX;
				pSelectedBall->py = m_mousePosY;
			}
		}

		if (m_mouse[0].bReleased)
		{
			pSelectedBall = nullptr;
		}

		if (m_mouse[1].bReleased)
		{
			if (pSelectedBall != nullptr)
			{
				// Apply velocity
				pSelectedBall->vx = 5.0f * ((pSelectedBall->px) - (float)m_mousePosX);
				pSelectedBall->vy = 5.0f * ((pSelectedBall->py) - (float)m_mousePosY);
			}

			pSelectedBall = nullptr;
		}


		vector<pair<sBall*, sBall*>> vecCollidingPairs;

		// Update Ball Positions
		for (auto &ball : vecBalls)
		{
			// Add Drag to emulate rolling friction
			ball.ax = -ball.vx * 0.8f;
			ball.ay = -ball.vy * 0.8f;

			// Update ball physics
			ball.vx += ball.ax * fElapsedTime;
			ball.vy += ball.ay * fElapsedTime;
			ball.px += ball.vx * fElapsedTime;
			ball.py += ball.vy * fElapsedTime;

			// Wrap the balls around screen
			if (ball.px < 0) ball.px += (float)ScreenWidth();
			if (ball.px >= ScreenWidth()) ball.px -= (float)ScreenWidth();
			if (ball.py < 0) ball.py += (float)ScreenHeight();
			if (ball.py >= ScreenHeight()) ball.py -= (float)ScreenHeight();

			
			// Clamp velocity near zero
			if (fabs(ball.vx*ball.vx + ball.vy*ball.vy) < 0.01f)
			{
				ball.vx = 0;
				ball.vy = 0;
			}
		}

		// Static collisions, i.e. overlap
		for (auto &ball : vecBalls)
		{
			for (auto &target : vecBalls)
			{
				if (ball.id != target.id)
				{
					if (DoCirclesOverlap(ball.px, ball.py, ball.radius, target.px, target.py, target.radius))
					{
						// Collision has occured
						vecCollidingPairs.push_back({ &ball, &target });

						// Distance between ball centers
						float fDistance = sqrtf((ball.px - target.px)*(ball.px - target.px) + (ball.py - target.py)*(ball.py - target.py));

						// Calculate displacement required
						float fOverlap = 0.5f * (fDistance - ball.radius - target.radius);

						// Displace Current Ball away from collision
						ball.px -= fOverlap * (ball.px - target.px) / fDistance;
						ball.py -= fOverlap * (ball.py - target.py) / fDistance;

						// Displace Target Ball away from collision
						target.px += fOverlap * (ball.px - target.px) / fDistance;
						target.py += fOverlap * (ball.py - target.py) / fDistance;
					}
				}
			}
		}
		// Work out static collisions with walls and displace balls so no overlaps
		for (auto &ball : vecBalls)
		{
			//float fDeltaTime = ball.fSimTimeRemaining;

			// Against Edges
			for (auto &edge : vecLines)
			{
				// Check that line formed by velocity vector, intersects with line segment
				float fLineX1 = edge.ex - edge.sx;
				float fLineY1 = edge.ey - edge.sy;

				float fLineX2 = ball.px - edge.sx;
				float fLineY2 = ball.py - edge.sy;

				float fEdgeLength = fLineX1 * fLineX1 + fLineY1 * fLineY1;

				// This is nifty - It uses the DP of the line segment vs the line to the object, to work out
				// how much of the segment is in the "shadow" of the object vector. The min and max clamp
				// this to lie between 0 and the line segment length, which is then normalised. We can
				// use this to calculate the closest point on the line segment
				float t = max(0, min(fEdgeLength, (fLineX1 * fLineX2 + fLineY1 * fLineY2))) / fEdgeLength;

				// Which we do here
				float fClosestPointX = edge.sx + t * fLineX1;
				float fClosestPointY = edge.sy + t * fLineY1;

				// And once we know the closest point, we can check if the ball has collided with the segment in the
				// same way we check if two balls have collided
				float fDistance = sqrtf((ball.px - fClosestPointX)*(ball.px - fClosestPointX) + (ball.py - fClosestPointY)*(ball.py - fClosestPointY));

				if (fDistance <= (ball.radius + edge.radius))
				{
					// Collision has occurred - treat collision point as a ball that cannot move. To make this
					// compatible with the dynamic resolution code below, we add a fake ball with an infinite mass
					// so it behaves like a solid object when the momentum calculations are performed
					sBall *fakeball = new sBall();
					fakeball->radius = edge.radius;
					fakeball->mass = ball.mass * 0.8f;
					fakeball->px = fClosestPointX;
					fakeball->py = fClosestPointY;
					fakeball->vx = -ball.vx;	// We will use these later to allow the lines to impart energy into ball
					fakeball->vy = -ball.vy;	// if the lines are moving, i.e. like pinball flippers



					// Add collision to vector of collisions for dynamic resolution
					vecCollidingPairs.push_back({ &ball, fakeball });

					// Calculate displacement required
					float fOverlap = 1.0f * (fDistance - ball.radius - fakeball->radius);

					// Displace Current Ball away from collision
					ball.px -= fOverlap * (ball.px - fakeball->px) / fDistance;
					ball.py -= fOverlap * (ball.py - fakeball->py) / fDistance;
				}
			}
		}
		// Now work out dynamic collisions
		for (auto c : vecCollidingPairs)
		{
			sBall *b1 = c.first;
			sBall *b2 = c.second;

			// Distance between balls
			float fDistance = sqrtf((b1->px - b2->px)*(b1->px - b2->px) + (b1->py - b2->py)*(b1->py - b2->py));

			// Normal
			float nx = (b2->px - b1->px) / fDistance;
			float ny = (b2->py - b1->py) / fDistance;

			// Tangent
			float tx = -ny;
			float ty = nx;

			// Dot Product Tangent
			float dpTan1 = b1->vx * tx + b1->vy * ty;
			float dpTan2 = b2->vx * tx + b2->vy * ty;

			// Dot Product Normal
			float dpNorm1 = b1->vx * nx + b1->vy * ny;
			float dpNorm2 = b2->vx * nx + b2->vy * ny;

			// Conservation of momentum in 1D
			float m1 = (dpNorm1 * (b1->mass - b2->mass) + 2.0f * b2->mass * dpNorm2) / (b1->mass + b2->mass);
			float m2 = (dpNorm2 * (b2->mass - b1->mass) + 2.0f * b1->mass * dpNorm1) / (b1->mass + b2->mass);

			// Update ball velocities
			b1->vx = tx * dpTan1 + nx * m1;
			b1->vy = ty * dpTan1 + ny * m1;
			b2->vx = tx * dpTan2 + nx * m2;
			b2->vy = ty * dpTan2 + ny * m2;

			// Wikipedia Version - Maths is smarter but same
			//float kx = (b1->vx - b2->vx);
			//float ky = (b1->vy - b2->vy);
			//float p = 2.0 * (nx * kx + ny * ky) / (b1->mass + b2->mass);
			//b1->vx = b1->vx - p * b2->mass * nx;
			//b1->vy = b1->vy - p * b2->mass * ny;
			//b2->vx = b2->vx + p * b1->mass * nx;
			//b2->vy = b2->vy + p * b1->mass * ny;
		}

		// Clear Screen
		Fill(0, 0, ScreenWidth(), ScreenHeight(), ' ');

		// Draw Balls
		for (auto ball : vecBalls)
			DrawWireFrameModel(modelCircle, ball.px, ball.py, atan2f(ball.vy, ball.vx), ball.radius, FG_WHITE);

		// Draw static collisions
		for (auto c : vecCollidingPairs)
			DrawLine(c.first->px, c.first->py, c.second->px, c.second->py, PIXEL_SOLID, FG_RED);

		// Draw Cue
		if (pSelectedBall != nullptr)
			DrawLine(pSelectedBall->px, pSelectedBall->py, m_mousePosX, m_mousePosY, PIXEL_SOLID, FG_BLUE);
		// Draw Lines
		for (auto line : vecLines)
		{
			FillCircle(line.sx, line.sy, line.radius, PIXEL_HALF, FG_WHITE);
			FillCircle(line.ex, line.ey, line.radius, PIXEL_HALF, FG_WHITE);

			float nx = -(line.ey - line.sy);
			float ny = (line.ex - line.sx);
			float d = sqrt(nx*nx + ny * ny);
			nx /= d;
			ny /= d;

			DrawLine((line.sx + nx * line.radius), (line.sy + ny * line.radius), (line.ex + nx * line.radius), (line.ey + ny * line.radius));
			DrawLine((line.sx - nx * line.radius), (line.sy - ny * line.radius), (line.ex - nx * line.radius), (line.ey - ny * line.radius));
		}

		return true;
	}

};


int main()
{
	CirclePhysics game;
	if (game.ConstructConsole(160, 120, 8, 8))
		game.Start();
	else
		wcout << L"Could not construct console" << endl;

	return 0;
};