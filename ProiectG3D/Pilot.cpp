#include "Pilot.h"

/*Pilot::Pilot(std::string const& path, bool bSmoothNormals, bool gamma)
    : Model(path, bSmoothNormals, gamma), _offset(0.0f, 1.2f, 0.0f) // Offset implicit pentru pilot
{
}

void Pilot::UpdatePosition(const glm::vec3& kartPosition)
{
    _rootTransf = glm::translate(glm::mat4(1.0f), kartPosition + _offset);
}

void Pilot::Draw(Shader& shader)
{
    shader.setMat4("model", _rootTransf);
    Model::Draw(shader);
}
*/



// Constructorul clasei Pilot
Pilot::Pilot(std::string const& path, bool bSmoothNormals, bool gamma)
    : Model(path, bSmoothNormals, gamma), _offset(3.0f, 0.0f, -1.0f) // Ajustează înălțimea pentru poziție
{
}

// Metodă pentru a seta poziția pilotului în funcție de poziția kart-ului
void Pilot::UpdatePosition(const glm::vec3& kartPosition) {
    // Resetăm matricea de transformare
    _rootTransf = glm::mat4(1.0f);

    // Aplicăm translația bazată pe poziția kart-ului și pe offset
    _rootTransf = glm::translate(_rootTransf, kartPosition + _offset);

    // Aplicăm rotația pentru a alinia pilotul drept în kart
   // _rootTransf = glm::rotate(_rootTransf, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    _rootTransf = glm::rotate(_rootTransf, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
 //   _rootTransf = glm::rotate(_rootTransf, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));


    // Aplicăm scalarea pentru a micșora pilotul mai mult
    _rootTransf = glm::scale(_rootTransf, glm::vec3(0.1f));
}
void Pilot::Draw(Shader& shader)
{
    shader.setMat4("model", _rootTransf);
    Model::Draw(shader);
}

