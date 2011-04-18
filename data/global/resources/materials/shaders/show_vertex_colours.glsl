/// Simple pixel shader to show only vertex colours

in vec4 vColour;

out vec4 FragmentColour;

void main(void)
{
	FragmentColour = vColour;
}

