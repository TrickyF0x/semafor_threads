#include <string> //строки
#include <iostream> //ввод вывод
#include <fstream> //файлы
#include <unistd.h> //процессы
#include <pthread.h> //потоки
#include <sys/wait.h> //ожидание
#include <algorithm> //для работы векторов с файлом
#include <experimental/filesystem> //работа с файловой системой
#include <semaphore.h> //SEMAFORY
 
sem_t semaphore;
 
static int counter = 0, tim = 0;

namespace fs = std::experimental::filesystem;

struct soderjimoe
{
	std::string text;
};
 
std::string spath() // ЧТЕНИЕ ИМЕНИ ФАЙЛА
{
	std::string pth;
	std::cout << "Enter path to directory: ";
	std::getline(std::cin, pth);
	return pth;
}

void *subprogramm_starter(void *s) //запуск 1 программы
{
	int local = 0;
	sem_wait(&semaphore);
	soderjimoe *txt = (soderjimoe*) s;
	std::string stroka = txt->text;

	pid_t pr;
	pr = fork();
	int status;
	if(pr == 0)
	{
		execl("controlsum", " ", stroka.c_str(), NULL);
	}
	else if (pr > 0)
	{
		wait(&status);
	}

	if(counter > 0)
		local--;
	else if(counter < 0) local++;
	counter = local;
	tim++;
	sleep(1);
	sem_post(&semaphore);
}


int main()
{
	int rc;
	void* status;

	//инициализация вектора
    std::string path = spath();
    auto it = fs::directory_iterator(path);
    std::vector<fs::path> array;
    std::copy_if(fs::begin(it), fs::end(it), std::back_inserter(array), 
    [](const auto& entry){
        return fs::is_regular_file(entry);
    });

    //запись списка в файл
    std::string filename = "./list.txt";
    std::fstream stream(filename, stream.trunc | stream.in | stream.out);
    if (!stream.is_open()) 
    {
        std::cout << "File error " << filename << std::endl;
    } 
    else 
    {
        for (auto & p : array)
            stream << p.string() << std::endl;
    }	

///////////подсчет количества файлов////////////////
	char *stro = new char [1024];
    int j = 0;
    std::ifstream base("./list.txt");
    while (!base.eof())
    {
        base.getline(stro, 1024, '\n');
        j++;
    }
    base.close();
    delete stro;
///////////////////////////////////////////////////

    int num_threads = j - 1;
    j = 1;

	pthread_t thread[num_threads];
	pthread_attr_t attr;

	pthread_attr_init(&attr);
   	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	std::string s[num_threads]; // сюда будем класть считанные строки
	std::ifstream file("./list.txt"); // файл из которого читаем
	while(j <= num_threads) //запускаем потоки
	{
		getline(file, s[j-1]);
		j++;
	} file.close();

	sem_init(&semaphore, 0, 1);
	for(int i = 1; i <= num_threads; ++i) //SOZDAEM POTOKI
	{
		soderjimoe arg;
		arg.text = s[i-1];

		std::cout << "Thread " << i << " created" << std::endl;
		rc = pthread_create(&thread[i-1], &attr, subprogramm_starter, (void*) &arg);	
		if (rc) 
		{
         std::cout << "Error:unable to create thread, " << rc << std::endl;
         exit(-1);
      	}

      	pthread_join(thread[i-1], &status);
      	
		j++;
	}

	std::cout << "Complete: exit semaphore code (" << counter << "),  "<< "up time "<< tim << " sec.\n"<< "Total files checked " << num_threads << std::endl;
	sem_destroy(&semaphore);
	pthread_attr_destroy(&attr);    

    return 0;
}