#pragma once
#include "Defines.h"

float DotProduct(Vertex v1, Vertex v2)
{
	return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) + (v1.w * v2.w);
}


Matrix4x MultiplyMatrixByMatrix(const Matrix4x& Mat1, const Matrix4x& Mat2)
{
	Matrix4x allMat;
	
	allMat.rowX.x = DotProduct(Mat1.rowX, Mat2.ColumnX());
	allMat.rowX.y = DotProduct(Mat1.rowX, Mat2.ColumnY());
	allMat.rowX.z = DotProduct(Mat1.rowX, Mat2.ColumnZ());
	allMat.rowX.w = DotProduct(Mat1.rowX, Mat2.ColumnW());
	allMat.rowY.x = DotProduct(Mat1.rowY, Mat2.ColumnX());
	allMat.rowY.y = DotProduct(Mat1.rowY, Mat2.ColumnY());
	allMat.rowY.z = DotProduct(Mat1.rowY, Mat2.ColumnZ());
	allMat.rowY.w = DotProduct(Mat1.rowY, Mat2.ColumnW());
	allMat.rowZ.x = DotProduct(Mat1.rowZ, Mat2.ColumnX());
	allMat.rowZ.y = DotProduct(Mat1.rowZ, Mat2.ColumnY());
	allMat.rowZ.z = DotProduct(Mat1.rowZ, Mat2.ColumnZ());
	allMat.rowZ.w = DotProduct(Mat1.rowZ, Mat2.ColumnW());
	allMat.rowW.x = DotProduct(Mat1.rowW, Mat2.ColumnX());
	allMat.rowW.y = DotProduct(Mat1.rowW, Mat2.ColumnY());
	allMat.rowW.z = DotProduct(Mat1.rowW, Mat2.ColumnZ());
	allMat.rowW.w = DotProduct(Mat1.rowW, Mat2.ColumnW());
	return allMat;
}


Vertex MultiplyMatrixByVertex(const Matrix4x& mat, const Vertex& v) {
	Vertex result;
	result.x = DotProduct(mat.ColumnX(), v);
	result.y = DotProduct(mat.ColumnY(), v);
	result.z = DotProduct(mat.ColumnZ(), v);
	result.w = DotProduct(mat.ColumnW(), v);
	return result;
}

PointXY NDC2Screen(Vertex& point) {
	PointXY screenPoint;
	screenPoint.x = static_cast<int>((point.x + 1.0f) * (.5f * Screen_Width));
	screenPoint.y = static_cast<int>((1.0f - point.y) * (.5f * Screen_Height));
	return screenPoint;
}


Matrix4x OrthoInv(const Matrix4x& mat) {
	Matrix4x result;
	result.rowX = { mat.rowX.x, mat.rowY.x, mat.rowZ.x, 0.0f };
	result.rowY = { mat.rowX.y, mat.rowY.y, mat.rowZ.y, 0.0f };
	result.rowZ = { mat.rowX.z, mat.rowY.z, mat.rowZ.z, 0.0f };
	result.rowW = { 0.0f, 0.0f, 0.0f, 1.0f };
	Vertex origTranslation = { mat.rowW.x, mat.rowW.y, mat.rowW.z, 0.0f };
	result.rowW.x = -DotProduct(mat.rowX, origTranslation);
	result.rowW.y = -DotProduct(mat.rowY, origTranslation);
	result.rowW.z = -DotProduct(mat.rowZ, origTranslation);
	return result;
}


Matrix4x YRotationMatrix(float angle)
{
	Matrix4x mat;
	float radAng = angle * 3.14 / 180.0f;
	mat.rowX = { cos(radAng), 0, sin(radAng), 0 };
	mat.rowY = { 0, 1, 0, 0 };
	mat.rowZ = { -sin(radAng),0, cos(radAng), 0 };
	mat.rowW = { 0, 0, 0, 1 };
	return mat;
}

Matrix4x XRotationMatrix(float angle) {
	Matrix4x mat;
	float radAng = angle * 3.14 / 180.0f;
	mat.rowX = { 1, 0, 0, 0 };
	mat.rowY = { 0, cos(radAng), -sin(radAng), 0 };
	mat.rowZ = { 0, sin(radAng), cos(radAng), 0 };
	mat.rowW = { 0, 0, 0, 1 };
	return mat;
}


Matrix4x ZRotationMatrix(float angle) {
	Matrix4x mat;
	float radAng = angle * 3.14 / 180.0f;
	mat.rowX = { cos(radAng), -sin(radAng), 0, 0 };
	mat.rowY = { sin(radAng), cos(radAng), 0, 0 };
	mat.rowZ = { 0, 0, 1, 0 };
	mat.rowW = { 0, 0, 0, 1 };
	return mat;
}


Matrix4x Identity() {
	Matrix4x mat;
	mat.rowX = { 1, 0, 0, 0 };
	mat.rowY = { 0, 1, 0, 0 };
	mat.rowZ = { 0, 0, 1, 0 };
	mat.rowW = { 0, 0, 0, 1 };
	return mat;
}


Matrix4x Projection(float nearPln, float farPln, float vfov, float aspectRat) {
	Matrix4x mat = Identity();
	float radVFOV = vfov * 3.14 / 180.0f;
	float yScale = tan(radVFOV / 2);
	float xScale = yScale / aspectRat;
	mat.rowX.x = xScale;
	mat.rowY.y = 1 / yScale;
	mat.rowZ.z = (farPln + nearPln) / (nearPln - farPln);
	mat.rowZ.w = 1;
	mat.rowW.z = (2.0f * farPln * nearPln) / (nearPln - farPln);;
	mat.rowW.w = 0;
	return mat;
}

Matrix4x Translation(float x, float y, float z, const Matrix4x& mat) {
	Matrix4x res = Identity();
	res.rowW.x = x;
	res.rowW.y = y;
	res.rowW.z = z;
	res = MultiplyMatrixByMatrix(res, mat);
	return res;
}


Vertex Perspective(Vertex& v) {
	v.x /= v.w;
	v.y /= v.w;
	v.z /= v.w;

	return v;
}
