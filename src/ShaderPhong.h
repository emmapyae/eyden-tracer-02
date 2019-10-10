#pragma once

#include "ShaderFlat.h"

class CScene;

class CShaderPhong : public CShaderFlat
{
public:
	/**
	 * @brief Constructor
	 * @param scene Reference to the scene
	 * @param color The color of the object
	 * @param ka The ambient coefficient
	 * @param kd The diffuse reflection coefficients
	 * @param ks The specular refelection coefficients
	 * @param ke The shininess exponent
	 */
	CShaderPhong(CScene& scene, Vec3f color, float ka, float kd, float ks, float ke)
		: CShaderFlat(color)
		, m_scene(scene)
		, m_ka(ka)
		, m_kd(kd)
		, m_ks(ks)
		, m_ke(ke)
	{}
	virtual ~CShaderPhong(void) = default;

	virtual Vec3f Shade(const Ray& ray) const override
	{
		// got inspired from https://www.scratchapixel.com/lessons/3d-basic-rendering/phong-shader-BRDF
		Vec3f p_ambient = m_ka * CShaderFlat::Shade(ray); // the result of specular terms
		Vec3f sum_d, sum_s = 0; // initialize for the sum of difussion terms

		Ray D_ray;  // incident ray for the diffusion part
		// to calculate the sum of Ll (Il·N) 
		for (int i = 0;i < m_scene.m_vpLights.size(); i++)
		{
			D_ray.org = ray.org + ray.t * ray.dir;

			std::optional<Vec3f> LD = m_scene.m_vpLights[i]->Illuminate(D_ray); 
			// get the light intensity for the incident ray

			D_ray.t = std::numeric_limits<float>::infinity();
			if (!m_scene.Occluded(D_ray)) // continue one the light is not blocked
			{
				if(LD)
				{
					// multiply I and N & check if I.N > 0
					float I_N = max(0.0f, D_ray.dir.dot(ray.hit->GetNormal(ray)));
					sum_d += * LD * I_N;
					// sum up the products of each light
				}
			}
		}

		// to calculate kdcd Σl=0n-1 Ll(Il·N) 
		Vec3f p_diffuse = m_kd * sum_d.mul(CShaderFlat::Shade(ray));

		Ray S_ray; // incident ray for the specular part
		for(int i = 0;i < m_scene.m_vpLights.size(); i++)
		{
			S_ray.org = ray.org + ray.t * ray.dir;

			std::optional<Vec3f> LS = m_scene.m_vpLights[i]->Illuminate(S_ray);
			// get the light intensity for the incident ray

			S_ray.t = std::numeric_limits<float>::infinity();
			if (!m_scene.Occluded(S_ray)) // continue one the light is not blocked
			{
				if (LS)
				{
					// Implement  R = I - 2*(I.N)*N
					Vec3f R = S_ray.dir - 2*(S_ray.dir.dot(ray.hit->GetNormal(ray)))*ray.hit->GetNormal(ray);
					// Multiply I and R & check if I.N > 0
					float I_R = max(0.0f, ray.dir.dot(R));
					float ke = pow(I_R, m_ke); // calculate (Il·R)^ke
					sum_s += * LS * ke;
					// sum up the products of each light
				}
			}
		}
		Vec3f p_specular = m_ks * RGB(1,1,1).mul(sum_s);
		// cs = (1, 1, 1) for white highlights according to the description  
		// the result of specular terms
		
		Vec3f L_result = p_ambient + p_diffuse + p_specular;
		return L_result;
		// return RGB(0, 0, 0);
	}

	
private:
	CScene& m_scene;
	float 	m_ka;    ///< ambient coefficient
	float 	m_kd;    ///< diffuse reflection coefficients
	float 	m_ks;    ///< specular refelection coefficients
	float 	m_ke;    ///< shininess exponent
};
