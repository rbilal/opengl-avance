#version 330
in vec3 vViewSpacePosition; // Position du sommet dans l'espace view
in vec3 vViewSpaceNormal; // Normale du sommet transformé dans l'espace view
in vec2 vTexCoords; // Coordonnées de texture du sommet

out vec3 fFragColor;

void main() {	
	fFragColor = normalize(vViewSpaceNormal);
};
