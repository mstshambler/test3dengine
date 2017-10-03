#define MAX_GRID_CELLS 64
#define MAX_GRID_PATH_CELLS 16

#define TILE_SIZE 16
#define TILE_HEIGHT 16

#define ENTITY_HEIGHT 6
#define ENTITY_RADIUS 2

#define MAX_PARTICLES 1000

void debug(const char *fmt, ...);

typedef unsigned char byte;

typedef float color[4];

#ifdef WIN32
#define strcasecmp stricmp
#endif

// List
template <class T> 
class List;

template <class T>
class ListReader {
friend class List<T>;
public:
	ListReader(List<T> *l, int isLocal) {
		cur_element = NULL;
		cur_element_pos = -1;
		baseList = l;
		next_element = NULL;
		prev_element = NULL;
		next_element_pos = -1;
		prev_element_pos = -1;
		checkDir = 0;
		islocalReader = isLocal;
		isCreated = 0;

		if (baseList) {
			baseList->addToReaders(this);
			isCreated = 1;
		}
	}

	ListReader(int isLocal=1) {
		cur_element = NULL;
		cur_element_pos = -1;
		next_element = NULL;
		prev_element = NULL;
		baseList = NULL;
		next_element_pos = -1;
		prev_element_pos = -1;
		checkDir = 0;
		islocalReader = isLocal;
		isCreated = 0;
	}

	~ListReader() {
		if (baseList)
			baseList->removeFromReaders(this);
	}


	void attach(List<T> *l) {
		cur_element = NULL;
		cur_element_pos = -1;
		next_element = NULL;
		prev_element = NULL;
		next_element_pos = -1;
		prev_element_pos = -1;
		checkDir = 0;
		if(baseList!=l) {
			if(baseList) {
				baseList->removeFromReaders(this);
			} else if (!islocalReader && isCreated == 0){
				isCreated = 1;
			}
			baseList = l;
			if (baseList)
				baseList->addToReaders(this);
		}
	}

	void detach() {
		if(baseList) {
			baseList->removeFromReaders(this);
			baseList = NULL;
		}
	}

	int getCount() {
		if (baseList)
			return baseList->count;
		else
			return -1;
	}

	T *getFirstElement() {
		if (!baseList)
			return NULL;

		checkDir = 0;

		cur_element = baseList->first_element;
		if (cur_element) {
			cur_element_pos = 0;
			return cur_element->data;
		}
		cur_element_pos = -1;
		return NULL;
	}

	T *getLastElement() {
		if (!baseList)
			return NULL;

		checkDir = 0;

		cur_element = baseList->last_element;

		if (cur_element) {
			cur_element_pos = baseList->count-1;
			return cur_element->data;
		}
		cur_element_pos = -1;
		return NULL;
	}

	T *getNextElement(int pos=1) {
		if (!baseList)
			return NULL;

		if (cur_element) {
			cur_element = cur_element->next;
			cur_element_pos++;
		} else {
			if (checkDir) {
				cur_element = next_element;
				cur_element_pos = next_element_pos;
			} else {
				cur_element = baseList->first_element;
				if (cur_element)
					cur_element_pos = 0;
				else
					cur_element_pos = -1;
			}
		}
		checkDir = 0;
			
		pos--;
		while(cur_element && pos>0) {
			cur_element = cur_element->next;
			cur_element_pos++;
			pos--;
		}
		if (cur_element)
			return cur_element->data;
		cur_element_pos = -1;
		return NULL;
	}

	T *getPrevElement(int pos=1) {
		if (!baseList)
			return NULL;

		if (cur_element) {
			cur_element = cur_element->prev;
			cur_element_pos--;
		} else {
			if (checkDir) {
				cur_element = prev_element;
				cur_element_pos = prev_element_pos;
			} else {
				cur_element = baseList->last_element;
				if (cur_element)
					cur_element_pos = baseList->count-1;
				else
					cur_element_pos = -1;
			}
		}
		checkDir = 0;

		pos--;
		while(cur_element && pos>0) {
			cur_element = cur_element->prev;
			cur_element_pos--;
			pos--;
		}
		if (cur_element)
			return cur_element->data;
		cur_element_pos = -1;
		return NULL;
	}

	T *getElement(int pos) {
		int i;
		if (!baseList)
			return NULL;

		checkDir = 0;
		cur_element = baseList->first_element;
		i=0;
		while(i<pos && cur_element) {
			cur_element = cur_element->next;
			i++;
		}
		if (cur_element) {
			cur_element_pos = pos;
			return cur_element->data;
		}
		cur_element_pos = -1;
		return NULL;
	}

	T *getElement(T *element) {
		int pos;
		if (!baseList)
			return NULL;

		checkDir = 0;
		pos = 0;
		cur_element = baseList->first_element;
		while(cur_element && cur_element->data != element) {
			cur_element = cur_element->next;
			pos++;
		}
		if (cur_element) {
			cur_element_pos = pos;
			return cur_element->data;
		}
		cur_element_pos = -1;
		return NULL;
	}

	int getCurElement() {
		return cur_element_pos;
	}

	T *getCurElementData() {
		if (cur_element)
			return cur_element->data;
		return NULL;
	}


protected:
	int checkDir, islocalReader;
	List<T> *baseList;
	struct List<T>::list_t *cur_element, *next_element, *prev_element;
	int cur_element_pos, next_element_pos, prev_element_pos;
	int isCreated;
};

template <class T> 
class List {
friend class ListReader<T>;
public:
	List() {
		readersCount = createdReadersCount = 0;
		first_element = last_element = NULL; 
		count = 0;
		modified = 0;

		first_reader = last_reader = created_readers = NULL;
	}

	~List() {
		list_readers_t *lr, *lr2;
		clear(0);
		
		lr = first_reader;
		while(lr) {
			lr->listReader->baseList = NULL;
			lr->listReader->cur_element = NULL;
			lr->listReader->cur_element_pos = -1;
			lr2 = lr->next;
			delete lr;
			lr = lr2;
		}
		first_reader = last_reader = NULL;
		readersCount = 0;
	}

	void clear(int type=0) {
		list_readers_t *lr;

		while(first_element) {
			last_element = first_element->next;
			if (type==1)
				delete first_element->data;
			delete first_element;
			first_element = last_element;
		}
		first_element = last_element = NULL;
		count = 0;
		
		lr = first_reader;
		while(lr) {
			lr->listReader->cur_element = NULL;
			lr->listReader->cur_element_pos = -1;
			lr = lr->next;
		}
	}

	void addElement(T *element, int pos=-1) {
		list_t *new_element;
		list_readers_t *lr;

		new_element = new list_t();
		new_element->data = element;
		new_element->prev = NULL;
		new_element->next = NULL;
		if (first_element==NULL) {
			first_element = new_element;
			last_element = new_element;
		} else {
			if (pos<0 || pos==count) {
				last_element->next = new_element;
				new_element->prev = last_element;
				last_element = new_element;
			} else if (pos==0) {
				new_element->next = first_element;
				first_element->prev = new_element;
				first_element = new_element;

				lr = first_reader;
				while(lr) {
					if (lr->listReader->cur_element_pos>=0)
						lr->listReader->cur_element_pos++;
					lr = lr->next;
				}
			} else {
				int i;
				list_t *find_element;

				find_element = first_element;
				i = 0;
				while(i<pos-1 && find_element) {
					i++;
					find_element = find_element->next;
				}
				if (find_element) {
					new_element->next = find_element->next;
					new_element->prev = find_element;
					if (find_element->next)
						find_element->next->prev = new_element;
					find_element->next = new_element;

					lr = first_reader;
					while(lr) {
						if (lr->listReader->cur_element_pos>i)
							lr->listReader->cur_element_pos++;
						lr = lr->next;
					}
				}
			}
		}
		count++;
		modified = 1;
	}

	void removeElement(T *element, int del_data=0) {
		list_t *find_element, *prev_element;
		list_readers_t *lr;
		int i = 0;

		//debug(DEBUG_MODE_TEST, "%s %i count %i", __FUNCTION__, first_element, count);
		find_element = first_element;
		prev_element = NULL;
		while(find_element && find_element->data != element) {
			prev_element = find_element;
			find_element = find_element->next;
			i++;
		}
		if (find_element) {
			if (find_element == first_element) {
				first_element = first_element->next;
				if (first_element)
					first_element->prev = NULL;
				else
					last_element = NULL;
			}
			if (find_element == last_element) {
				last_element = last_element->prev;
				if (last_element)
					last_element->next = NULL;
			}
			if (prev_element) {
				prev_element->next = find_element->next;
				if (find_element->next)
					find_element->next->prev = prev_element;
			}
			//debug(DEBUG_MODE_TEST, "%s2 %i", __FUNCTION__, first_element);
			lr = first_reader;
			while(lr) {
				if (lr->listReader->cur_element == find_element) {
					lr->listReader->checkDir = 1;

					lr->listReader->next_element = find_element->next;
					lr->listReader->next_element_pos = i;
					lr->listReader->prev_element = find_element->prev;
					lr->listReader->next_element_pos = i-1;

					lr->listReader->cur_element = NULL;
					lr->listReader->cur_element_pos = -1;
				} else if (lr->listReader->cur_element_pos>=i) {
					lr->listReader->cur_element_pos--;
				}
				lr = lr->next;
			}

			if (del_data)
				delete find_element->data;
			delete find_element;
			count--;
			modified = 1;
			//debug(DEBUG_MODE_TEST, "%s3 %i, count %i", __FUNCTION__, first_element, count);
		}
	}

	void removeElement(int pos, int del_data=0) {
		int i;
		list_t *find_element, *prev_element;
		list_readers_t *lr;

		find_element = first_element;
		prev_element = NULL;
		i = 0;
		while(i<pos && find_element) {
			prev_element = find_element;
			find_element = find_element->next;
			i++;
		}
		if (find_element) {
			if (find_element == first_element) {
				first_element = first_element->next;
				if (first_element)
					first_element->prev = NULL;
				else
					last_element = NULL;
			}
			if (find_element == last_element) {
				last_element = last_element->prev;
				if (last_element)
					last_element->next = NULL;
			}
			if (prev_element) {
				prev_element->next = find_element->next;
				if (find_element->next)
					find_element->next->prev = prev_element;
			}

			lr = first_reader;
			while(lr) {
				if (lr->listReader->cur_element == find_element) {
					lr->listReader->checkDir = 1;

					lr->listReader->next_element = find_element->next;
					lr->listReader->next_element_pos = i;
					lr->listReader->prev_element = find_element->prev;
					lr->listReader->next_element_pos = i-1;

					lr->listReader->cur_element = NULL;
					lr->listReader->cur_element_pos = -1;
				} else if (lr->listReader->cur_element_pos>=i) {
					lr->listReader->cur_element_pos--;
				}
				lr = lr->next;
			}

			if (del_data)
				delete find_element->data;
			delete find_element;
			count--;
			modified = 1;
		}
	}

	void removeElementsFrom(int pos, int del_data=0) {
		int i;
		list_t *find_element, *next_element;
		list_readers_t *lr;

		if (pos==0) {
			clear(del_data);
			return;
		}

		find_element = first_element;
		i = 0;
		while(i<pos && find_element) {
			find_element = find_element->next;
			i++;
		}
		if (find_element) {
			last_element = find_element->prev;
			last_element->next = NULL;

			while(find_element) {
				next_element = find_element->next;

				lr = first_reader;
				while(lr) {
					if (lr->listReader->cur_element == find_element) {
						lr->listReader->cur_element = NULL;
						lr->listReader->cur_element_pos = -1;
					} else if (lr->listReader->cur_element_pos>=i) {
						lr->listReader->cur_element_pos--;
					}
					lr = lr->next;
				}

				if (del_data)
					delete find_element->data;
				delete find_element;

				count--;
				find_element = next_element;
			}

			modified = 1;
		}
	}

	int getCount() {
		return count;
	}

	int getElementIndex(T *element) { //new by den, 06.21
		int pos=0;

		cur_element = first_element;
		while(cur_element && cur_element->data != element) {
			cur_element = cur_element->next;
			pos++;
		}
		if (cur_element) {
			return pos;
		}
		return -1;
	}

	int getModified() {return modified;}
	void setModified(int i) {modified = i;}
	
protected:
	struct list_t {
		T *data;
		list_t *next, *prev;
	};

	struct list_readers_t {
		ListReader<T> *listReader;
		list_readers_t *next;
	};

	list_t *first_element, *last_element, *cur_element;
	int count,readersCount,createdReadersCount;
	int modified;
	list_readers_t *first_reader, *last_reader, *created_readers;

	list_t *findElementByPos(int pos) {
		int i;
		list_t *find_element;

		find_element = first_element;
		i=0;
		while(i<pos && find_element) {
			find_element = find_element->next;
			i++;
		}
		return find_element;
	}

	list_t *findElementByData(T *element) {
		list_t *find_element;

		find_element = first_element;
		while(find_element && find_element->data != element) {
			find_element = find_element->next;
		}
		return find_element;
	}

	void addToReaders(ListReader<T> *i) {
		list_readers_t *new_reader;

		new_reader = new list_readers_t();
		new_reader->listReader = i;
		if (first_reader==NULL) {
			new_reader->next = NULL;
			first_reader = new_reader;
			last_reader = new_reader;
		} else {
			new_reader->next = first_reader;
			first_reader = new_reader;
		}
		
		readersCount++;
	}

	void removeFromReaders(ListReader<T> *lr) {
		list_readers_t *cur_reader=NULL, *prev_reader=NULL;

		if (first_reader == NULL)
			return;

		cur_reader = first_reader;
		prev_reader = NULL;

		if (first_reader->listReader == lr) {
			first_reader = first_reader->next;
			if (first_reader == NULL)
				last_reader = NULL;
			delete cur_reader;
			readersCount--;
		} else {
			while(cur_reader && cur_reader->listReader != lr) {
				prev_reader = cur_reader;
				cur_reader = cur_reader->next;
			}
			if (cur_reader) {
				if (last_reader == cur_reader)
					last_reader = prev_reader;
				prev_reader->next = cur_reader->next;
				delete cur_reader;  //TODO: in other way
				readersCount--;
			}
		}
	}
};

template <class T> 
class Array {
public:
	T **data;
	long count;
	long maxitems;
	
	Array() {
		data = NULL;
		count = 0;
		maxitems = 0;
	}
	Array(long l) {
		data = new T*[l];
		count = 0;
		maxitems = l;
	}
	void Init(long l) {
		data = new T*[l];
		count = 0;
		maxitems = l;
	}
	void AddData(T *d) {
		if (count>=maxitems) return;
		data[count] = d;
		count++;
	}
	void RemoveDataByNum(long i) {
		if (i>=count) return;
		memmove(data+i, data+(i+1), (count-i-1)*sizeof(T));
		count--;
	}
	void RemoveData(T *d) {
		T **t;
		long i;

		t = data;
		i = 0;
		while(i<count && *t != d) {
			t++;
			i++;
		}
		RemoveDataByNum(i);
	}
	T *GetData(long i) {
		if (i>=count) return NULL;
		return data[i];
	}
	T *GetTopData() {
		if (count<=0) return NULL;
		return data[0];
	}
	T *GetLastData() {
		if (count<=0) return NULL;
		return data[count-1];
	}
	long GetCount() {
		return count;
	}
	void SetCount(long l) {
		count = l;
	}
};

#define dynamicReaderFree(name, class_t) static ListReader<class_t> *name = new ListReader<class_t>(NULL, 0)
