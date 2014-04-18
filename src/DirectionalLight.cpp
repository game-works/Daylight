#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(const std::string &lightName, const sf::Vector2f &initialPosition, const sf::Color &color, const float initailItensity, const float angleIn, const float openAngle)
       : Light(), lightKey(lightName), lightVector(initialPosition), lightColor(color), intensity(initailItensity), facingAngle(angleIn), openingAngle(openAngle)
{

    float fangle     = facingAngle * M_PI / 180;
    float offsetAngle   = openingAngle * M_PI / 180;
      // fangle -= offsetAngle;
    float radius = 200;

        sf::Vector2f l(cos(offsetAngle), sin(offsetAngle));
       for(float angle=fangle-offsetAngle/2;angle<fangle+offsetAngle/2;angle+=(M_PI*2)/20){
             directionalRays.push_back(sf::Vector2f(lightVector.x + (cos(angle )) ,lightVector.y + (sin(angle   ))  ));

        }

//    	for(float angle=offsetAngle;angle>offsetAngle/2;angle-=(M_PI*2)/20){
//             directionalRays.push_back(sf::Vector2f(lightVector.x + (cos(angle + fangle * 0.5)),lightVector.y + (sin(angle  + fangle))));
//        }


//    directionalRays.push_back(sf::Vector2f(lightVector.x + (cos(angle + offsetAngle)),lightVector.y + (sin(angle + offsetAngle ))));
//    directionalRays.push_back(sf::Vector2f( lightVector.x + (cos(angle - offsetAngle)), lightVector.y+(sin(angle - offsetAngle ))));


}

DirectionalLight::~DirectionalLight()
{
}


sf::Vector2f DirectionalLight::getVec()
{
    return lightVector;
}


std::string DirectionalLight::getKey()
{
    return lightKey;
}

void DirectionalLight::setVec(const sf::Vector2f &lightVec)
{
    lightVector = lightVec;
    float angle     = facingAngle * M_PI / 180;
    float offsetAngle   = openingAngle * M_PI / 180;
    float radius = 200;
    directionalRays.clear();
    directionalRays.push_back(sf::Vector2f(lightVector.x + (cos(angle + offsetAngle)),lightVector.y + (sin(angle + offsetAngle ))));
    directionalRays.push_back(sf::Vector2f( lightVector.x + (cos(angle - offsetAngle)), lightVector.y+(sin(angle - offsetAngle ))));
}

sf::Color DirectionalLight::getColor()
{
    return lightColor;
}

float DirectionalLight::getIntensity()
{
    return intensity;
}

bool DirectionalLight::compareIntersects(Intersect vec1, Intersect vec2)
{
    if(vec1.getAngle()  - vec2.getAngle() < 0)
    {
        return true;
    }
    else if (vec1.getAngle()  - vec2.getAngle() > 0)
    {
        return false;
    }

    return false;
}

Intersect DirectionalLight::getLineIntersect(sf::VertexArray ray, sf::VertexArray segment)
{

    float r_px= ray[0].position.x;
    float r_py = ray[0].position.y;
    float r_dx = ray[1].position.x - ray[0].position.x;
    float r_dy = ray[1].position.y - ray[0].position.y;

    float s_px = segment[0].position.x;
    float s_py =segment[0].position.y;
    float s_dx = segment[1].position.x - segment[0].position.x;
    float s_dy =segment[1].position.y - segment[0].position.y;

    //parallel check

    sf::Vector2f normalRay = VectorMath::normalize(sf::Vector2f(r_dx, r_dy));
    sf::Vector2f normalSeg = VectorMath::normalize(sf::Vector2f(s_dx, s_dy));
    float dot = VectorMath::dot(normalRay, normalSeg);

    if(dot == 0)  // if they point in same direction, then theres no intersection
    {
        return  Intersect(sf::Vector2f(0,0), 0);
    }

    float T2 = (r_dx*(s_py-r_py) + r_dy*(r_px-s_px))/(s_dx*r_dy - s_dy*r_dx);
    float T1 = (s_px+s_dx*T2-r_px)/r_dx;

    if(T1<0)
    {
        return Intersect(sf::Vector2f(0,0), 0);
    }
    if(T2<0 || T2>1)
    {
        return Intersect(sf::Vector2f(0,0), 0);
    }

    return Intersect(sf::Vector2f(r_px+r_dx*T1,r_py+r_dy*T1),T1);
}


std::vector<Intersect> DirectionalLight::getIntersectPoints( std::vector<sf::Vector2f> &shapeVectors, const std::vector<float> &uniqueAngles)
{
    std::vector<Intersect> intersects;
    for(int uniqueAngleIndex=0; uniqueAngleIndex < directionalRays.size(); uniqueAngleIndex++)
    {

        sf::VertexArray ray(sf::Lines);
        ray.append(sf::Vertex(lightVector, sf::Color::Black));
        ray.append(sf::Vertex(directionalRays[uniqueAngleIndex], sf::Color::Black));

        Intersect closestInterect(sf::Vector2f(799,799), 1000);
        for(int i = 0 ;  i < shapeVectors.size(); i+=2)
        {
            sf::Vector2f seg1 =shapeVectors[i];
            sf::Vector2f seg2 =shapeVectors[i+1];
            sf::VertexArray segLine(sf::Lines);
            segLine.append(seg1);
            segLine.append(seg2);
            Intersect intersect =  getLineIntersect(ray, segLine);

            if(intersect.getIntersectPoint().x > 0 && intersect.getIntersectPoint().y > 0)
            {
                //add only the intersect with the smallest magnitude since it will be closest intersect
                if(intersect.getParam() < closestInterect.getParam())
                {
                    closestInterect = intersect;
                }
            }

        }

        if(closestInterect.getParam() < 1000)
        {
           // closestInterect.setAngle(angle);
            intersects.push_back(closestInterect);
        }
    }
    std::sort(intersects.begin(), intersects.end(), compareIntersects);
    return intersects;
}

void DirectionalLight::generateLight(std::vector<sf::Vector2f> &shapePoints, std::vector<float> &uniqueAngles)
{

    std::vector<Intersect> intersects = getIntersectPoints(shapePoints,uniqueAngles);


     sf::VertexArray rayLine(sf::TrianglesFan);
    rayLine.append(sf::Vertex(lightVector, sf::Color::White));
    sf::VertexArray rays(sf::Lines);

    for(int i = 0; i < intersects.size(); i++)
    {
        rayLine.append(sf::Vertex(intersects[i].getIntersectPoint(), sf::Color::White));

        if(shouldDebugLines)
        {
            rays.append(sf::Vertex(lightVector, sf::Color::Red));
            rays.append(sf::Vertex(intersects[i].getIntersectPoint(), sf::Color::Red));
        }
    }

//    rayLine.append(sf::Vertex(lightVector, sf::Color::White));
//    rayLine.append(sf::Vertex(intersects[0].getIntersectPoint(), sf::Color::White));
//    rayLine.append(sf::Vertex(intersects[1].getIntersectPoint(), sf::Color::White));

//        if(shouldDebugLines)
//        {
//            rays.append(sf::Vertex(lightVector, sf::Color::Red));
//            rays.append(sf::Vertex(intersects[i].getIntersectPoint(), sf::Color::Red));
//        }


    lightVertexArray = rayLine;
    if(shouldDebugLines){
        debugRays = rays;
    }

}


void DirectionalLight::render(sf::RenderTarget &target, sf::RenderStates &renderState)
{

    target.draw(lightVertexArray, renderState);

    if(shouldDebugLines)
    {
      target.draw(debugRays);
    }

}