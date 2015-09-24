#ifndef _MATERIAL_H
#define _MATERIAL_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Material
{
private:
    glm::vec4 emission;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float shininess;
    float absorption,reflection,transparency,refractive_index;

public:
    Material(void)
    {
        init();
    }

    ~Material(void)
    {

    }

    void init()
    {
        setEmission(0.0f,0.0f,0.0f,1.0f);
        setAmbient(0.0f,0.0f,0.0f,1.0f);
        setDiffuse(0.0f,0.0f,0.0f,1.0f);
        setSpecular(0.0f,0.0f,0.0f,1.0f);
        setShininess(0.0f);
        setAbsorption(1);
        setReflection(0);
        setTransparency(0);
    }

    void setEmission(float r,float g,float b,float a=1)
    {
        emission[0] = r;
        emission[1] = g;
        emission[2] = b;
        emission[3] = a;
    }

    void setAmbient(float r,float g,float b,float a=1)
    {
        ambient[0] = r;
        ambient[1] = g;
        ambient[2] = b;
        ambient[3] = a;
    }

    void setDiffuse(float r,float g,float b,float a=1)
    {
        diffuse[0] = r;
        diffuse[1] = g;
        diffuse[2] = b;
        diffuse[3] = a;
    }

    void setSpecular(float r,float g,float b,float a=1)
    {
        specular[0] = r;
        specular[1] = g;
        specular[2] = b;
        specular[3] = a;
    }

    void setShininess(float r)
    {
        shininess = r;
    }

    void setAbsorption(float a)
    {
        absorption = a;
    }

    void setReflection(float r)
    {
        reflection = r;
    }

    void setTransparency(float t)
    {
        transparency = t;
        ambient[3] = diffuse[3] = specular[3] = emission[3] = 1-t;
    }

    void setRefractiveIndex(float r)
    {
        refractive_index = r;
    }

    const glm::vec4 getEmission() const
    {
        return emission;
    }

    const glm::vec4 getAmbient() const
    {
        return ambient;
    }

    const glm::vec4 getDiffuse() const
    {
        return diffuse;
    }

    const glm::vec4 getSpecular() const
    {
        return specular;
    }
    float getShininess() const
    {
        return shininess;
    }

    const float getAbsorption() const
    {
        return absorption;
    }

    const float getReflection() const
    {
        return reflection;
    }

    const float getTransparency() const
    {
        return transparency;
    }

    const float getRefractiveIndex() const
    {
        return refractive_index;
    }
};

#endif
