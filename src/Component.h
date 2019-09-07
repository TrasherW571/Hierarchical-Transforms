#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <memory>
using namespace std;

class Component
{
public:
	Component::Component(double Sx, double Sy, double Sz, double Tx, double Ty, double Tz, double Mx, double My, double Mz, Component* _parent = NULL, Component* _prev = NULL, Component* _next = NULL)
		:Scale_Factors(Sx, Sy, Sz), Trans_to_Joint(Tx, Ty, Tz), Trans_to_Mesh(Mx, My, Mz), parent(_parent), next(_next), prev(_prev), Joint_AngleX(1, 0, 0), Joint_AngleY(0, 1, 0), Joint_AngleZ(0, 0, 1)
	{
		if (parent)
		{
			parent->children.push_back(this);
		}
		angleX = 0;
		angleY = 0;
		angleZ = 0;
	}
	~Component()
	{
		for (int i = 0; i < children.size(); i++)
		{
			delete children.at(i);
		}
		delete this;
	}
	//Draw the component
	void Draw(shared_ptr<MatrixStack> &MV, shared_ptr<MatrixStack> &P, shared_ptr<Program> &prog, shared_ptr<Shape> &shape)
	{
		MV->pushMatrix();
		//Transform to Joint
		MV->translate(Trans_to_Joint);
		MV->rotate(angleX, Joint_AngleX);
		MV->rotate(angleY, Joint_AngleY);
		MV->rotate(angleZ, Joint_AngleZ);
		//Transform to Mesh
		MV->pushMatrix();
		MV->translate(Trans_to_Mesh);
		MV->scale(Scale_Factors);
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P->topMatrix()[0][0]);
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
		shape->draw(prog);
		prog->unbind();
		MV->popMatrix();
		//Draw Children
		for (int i = 0; i < children.size(); i++)
		{
			children.at(i)->Draw(MV, P, prog, shape);
		}
		MV->popMatrix();
	}
	//Adjusting Angle Functions
	void Adjust_X_Angle(double x)
	{
		angleX += x;
		glm::vec3 &Joint = Joint_AngleX;
		Joint += glm::vec3(1, 0, 0);
	}
	void Adjust_Y_Angle(double y)
	{
		angleY += y;
		glm::vec3 &Joint = Joint_AngleY;
		Joint += glm::vec3(0, 1, 0);
	}
	void Adjust_Z_Angle(double z)
	{
		angleZ += z;
		glm::vec3 &Joint = Joint_AngleZ;
		Joint += glm::vec3(0, 0, 1);
	}
	//Adjusting Scale
	void Adjust_Scale(double x)
	{
		glm::vec3 &Scale = Scale_Factors;
		Scale += glm::vec3(x, x, x);
	}
	//Setter Function
	void set_Next(Component* c)
	{
		next = c;
	}
	//Getter Functions
	Component* get_Next() { return next; }
	Component* get_Prev() { return prev; }
	glm::vec3 get_Scale() { return Scale_Factors; }
	Component* get_Parent() { return parent; }

private:
	Component* parent;
	vector<Component*> children;
	Component* next;
	Component* prev;
	glm::vec3 Joint_AngleX;
	glm::vec3 Joint_AngleY;
	glm::vec3 Joint_AngleZ;
	glm::vec3 Scale_Factors;
	glm::vec3 Trans_to_Joint;
	glm::vec3 Trans_to_Mesh;
	double angleX;
	double angleY;
	double angleZ;
};