#include "lectorcsv.h"
#include "lectorcsv.h"
#include <string>

lectorCSV::lectorCSV(char* filename, char* orange_filename)
{

    this->filename = filename;
    this->file.open(filename);

    this->orange_filename = orange_filename;
    this->orange_file.open(orange_filename);

    this->contador_nodos_verdes = 0;
    this->contador_nodos_naranjas = 0;

    if(!file)
    {
        std::cout << "FATAL ERROR: " << filename << " NOT FOUND IN DIRECTORY, ABORTING PROGRAM..." << std::endl;
    }
    else if(!orange_file)
    {
        std::cout << "FATAL ERROR: " << orange_filename << " NOT FOUND IN DIRECTORY, ABORTING PROGRAM..." << std::endl;
    }
    else
    {
        std::cout << "Leyendo CSV: " << this->filename << std::endl;
        read_graph_from_csv(); //Lee el grafo de nodos verdes según topología
        std::cout << "Leyendo orange file: " << this->orange_filename << std::endl;
        read_orange_neighbours_from_file();
        
        std::cout << "Finalice!" << std::endl;
    }

}


lectorCSV::~lectorCSV()
{

}

void lectorCSV::read_graph_from_csv()
{
    std::list<int> prueba;
    std::string line, word, temp, end_line;
    end_line = '\n';

    //Lee el archivo CSV con formato de fila: NODO,VECINO1,VECINO2, ... , VECINO N
    while( !(this->file.eof() ))
    {
        int num_elementos = 0;
        NODO_V temporal_node;
        int vecino = 0;

        //leer una fila completa y dejarla en "line"
        getline( file, line );
        std::stringstream stream(line);

        if( !line.empty() )
        {
            std::cout << "Linea " << contador_nodos_verdes << ": ";
            while(getline(stream, word, ','))
            {
                if(num_elementos == 0)
                {
                    temporal_node.name = std::stoi(word);
                    std::cout << "Key: (" << temporal_node.name << "), Vecinos:" ;
                }
                else
                {
                    vecino = std::stoi(word);
                    //this->grafo_v[temporal_node].push_back(vecino);
                    std::cout << " " << vecino ;
                }

                num_elementos++;
            }

            std::cout << std::endl;
            this->contador_nodos_verdes++;
        }
    }
    this->file.close();
    //show_map();
}

void lectorCSV::read_orange_neighbours_from_file()
{   
	
	std::string line;
	
	while( std::getline(this->orange_file, line) )
	{
		std::stringstream linestream(line);
		//std::cout << "Linea " << contador_nodos_naranjas << ": " << line << std::endl;
		
		std::string delimiter = ":";
		size_t pos = 0;
		std::string token, ip, port;
		
		while ((pos = line.find(delimiter)) != std::string::npos)
		{
			token = line.substr(0, pos);
			//std::cout << "IP: " << token;
			ip = token;
			line.erase(0, pos + delimiter.length());
		}
		
		//std::cout << " Puerto: " << line << std::endl;
		port = line;
		
		std::cout << "IP: " << ip << " Puerto: " << port << std::endl;
		
	}
	
	this->contador_nodos_naranjas++;
    this->orange_file.close();
}

int lectorCSV::get_num_nodos_verdes()
{
    return this->contador_nodos_verdes;
}

int lectorCSV::get_num_nodos_naranjas()
{
    return this->contador_nodos_naranjas;
}


void lectorCSV::show_map()
{
    std::map<NODO_V , std::list<int>>::iterator it;

    for (it = this->grafo_v.begin(); it != this->grafo_v.end(); ++it)
    {
        std::cout << it->first.name << ": ";

        std::list<int>::iterator list_it;

        for(list_it = it->second.begin(); list_it != it->second.end(); ++list_it)
        {
            std::cout << *list_it << " ";
        }

        std::cout << std::endl;

    }

    std::cout << "Contador nodos verdes: " << this->contador_nodos_verdes <<std::endl;
}

void lectorCSV::show_orange_neighbours()
{
    std::cout << "jaja" << std::endl;
}

