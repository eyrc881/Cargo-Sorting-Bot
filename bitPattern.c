unsigned int current;
unsigned int no_of_turns=3;

void traverse(int path)
{
	current=0;
	int i=3;
	int ePath=path-current;
	switch(ePath)
	{
		case 0:

			break;

		case 1:

			back();
			no_of_turns=1;
			break;

		case 9:
		case 10:
		case 11:

			back();
			back();
			no_of_turns=2;
			break;

		case 99:
		case 100:
		case 111:

			back();
			back();
			back();
			no_of_turns=3;
			break;
	}
	
	for (int i = 0; i < no_of_turns; ++i)
	{
		switch()
	}
}