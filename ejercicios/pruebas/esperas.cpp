#include <omp.h>
#include <iostream>

omp_lock_t writelock;

static char turno = 'J';

void pedro()
{
	while(turno == 'J')
		;

	omp_set_lock(&writelock);
	turno = 'J';
	std::cout << "Yo Pedro cedo el turno" << std::endl;
	omp_unset_lock(&writelock);
}

void juan()
{
	while(turno == 'P')
		;
	
	omp_set_lock(&writelock);
	turno = 'P';
	std::cout << "Yo Juan cedo el turno" << std::endl;
	omp_unset_lock(&writelock);

}

int main()
{
	omp_init_lock(&writelock);
	#pragma omp parallel num_threads(2)
	{
		int my_id = omp_get_thread_num();
		/*omp_set_lock(&writelock);
		std::cout << "Mi id es: " << my_id << std::endl;
		omp_unset_lock(&writelock);*/
		
		if(my_id == 0)
		{
			for(int i = 0; i < 3; ++i)
				pedro();
			
		}
		else
		{
			for(int i = 0; i < 3; ++i)
				juan();
			
		}
		
	}
	
	omp_destroy_lock(&writelock);
	
	return 0;
}
