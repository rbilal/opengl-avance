#version 330
in vec3 vPosition; // Position du sommet dans l'espace view
in vec3 vNormal; // Normale du sommet transformé dans l'espace view
in vec2 vTexCoords; // Coordonnées de texture du sommet

out vec3 fFragColor;

void main() {	
	fFragColor = normalize(vNormal);
};
