#include "mapreader.h"

#define handle_error(msg) \
  do { perror(msg); return(-1); } while (0)

int mapreader_init(int fd)
{
  if (fstat(fd, &voice_file_size) == -1)
    handle_error("fstat");

  voice_mapped_addr = mmap(NULL, voice_file_size.st_size, (PROT_READ | PROT_WRITE),MAP_PRIVATE, fd, 0);
  if (voice_mapped_addr == MAP_FAILED)
    {
      voice_mapped_addr = NULL;
      handle_error("mmap");
    }

  
  current_sought_addr = voice_mapped_addr;

  if( strcmp((char*)current_sought_addr,"CMU_FLITE_CG_VOXDATA-v1.001"))
    return -1;
  
  current_sought_addr += 28;

  return 0;

}

void mapreader_finish()
{
  if(voice_mapped_addr!=NULL)
    {
      munmap((void*)voice_mapped_addr, voice_file_size.st_size);
    }
}

char* padded_read_string()
{
  char* s;
  int numbytes;
  
  if(voice_mapped_addr == NULL) return NULL;
  
  numbytes = *((int*)current_sought_addr);
  //printf("reading %d bytes\n",numbytes);
  current_sought_addr += sizeof(int);
  
  s = (char*)current_sought_addr;

  current_sought_addr+=numbytes;
  
  return s;
}

cst_cg_db* mapreader_load_db()
{
  if(voice_mapped_addr == NULL) return NULL;

  cst_cg_db* db = malloc(sizeof(cst_cg_db));

  int numbytes = *((int*)current_sought_addr);
  current_sought_addr += sizeof(int);

  memcpy((void*)db, (void*)current_sought_addr, sizeof(cst_cg_db));
  current_sought_addr += sizeof(cst_cg_db);


  db->name = padded_read_string();

  //printf("Reading types...\n");
  db->types = (const char**)mapreader_read_types();

 //printf("Reading f0 trees...\n");
  db->f0_trees = (const cst_cart**) mapreader_read_tree_array();
  //printf("Reading param trees...\n");
  db->param_trees0 = (const cst_cart**) mapreader_read_tree_array();

  
  db->model_vectors0 = mapreader_read_2d_array();
  
  db->model_min = mapreader_read_array();
  db->model_range = mapreader_read_array();

  db->dur_stats = mapreader_read_dur_stats();
  db->dur_cart = mapreader_read_tree();

  db->phone_states = mapreader_read_phone_states();

  db->dynwin = mapreader_read_array();
  db->me_h = mapreader_read_2d_array();

  //printf("Value of range 4 = %1.4f\n",db->model_range[4]);
  //printf("Value of model vector 1 = %u\n",db->model_vectors0[10][0]);
  //printf("Value of ds 120: %s %1.4f %1.4f\n",db->dur_stats[120]->phone,db->dur_stats[120]->mean, db->dur_stats[120]->stddev);
  //printf("Value of ps 2 3: %s\n",db->phone_states[2][3]);
  //printf("me_h 2 3: %1.4f\n", db->me_h[2][3]);
  return db;
  
}

void mapreader_free_db(cst_cg_db* db)
{
  free(db);
}

void* mapreader_read_padded(int* numbytes)
{
  void* ret;
  *numbytes = *((int*)current_sought_addr);
  current_sought_addr += sizeof(int);
  ret = current_sought_addr;
  current_sought_addr += *numbytes;
  return ret;
}


char** mapreader_read_types()
{
  char** types;
  int numtypes;
  int i;

  //printf("mapreader read types\n");
  if(voice_mapped_addr == NULL) return NULL;
  
  numtypes = *((int*)current_sought_addr);
  current_sought_addr += sizeof(int);

  //printf("Number of types: %d\n",numtypes);
  types = current_sought_addr; //malloc((1+numtypes) * sizeof(char*));
  current_sought_addr += (numtypes+1)*sizeof(char*);
  
  for(i=0;i<numtypes;i++)
    {
      types[i] = padded_read_string();
    }
  types[i] = 0;
  
  return types;
}

cst_cart_node* mapreader_read_tree_node()
{
  int temp;
  cst_cart_node* node;
  cst_val** nodeval;

  void** nodeatom;

  node = mapreader_read_padded(&temp);
  nodeval = &(node->val);
  *nodeval = mapreader_read_padded(&temp);
  nodeatom = &(node->val->c.a.v.vval);
  if(node->val->c.a.type == CST_VAL_TYPE_STRING)
    {
      *nodeatom = mapreader_read_padded(&temp);
      //printf("read node for string: %s\n",node->val->c.a.v.vval);
    }
  return node;
}

cst_cart_node* mapreader_read_tree_nodes()
{
  cst_cart_node* nodes,*node;
  int temp;
  int i;
  cst_val** nodeval;
  void** nodeatom;


  nodes = mapreader_read_padded(&temp);
  //printf("%d\n",temp);

  // now read node data
  i=0;
  while(nodes[i].val!=0)
    {
      node = &(nodes[i]);
      nodeval = &(node->val);
      *nodeval = mapreader_read_padded(&temp);
      nodeatom = &(node->val->c.a.v.vval);

      if(node->val->c.a.type == CST_VAL_TYPE_STRING)
	{
	  *nodeatom = mapreader_read_padded(&temp);
	  //printf("read node for string: %s\n",node->val->c.a.v.vval);
	}
      i++;
      
    }

  return nodes;
}

char** mapreader_read_tree_feats()
{
  char** feats;
  int numfeats;
  int i;

  //printf("mapreader read feats\n");
  if(voice_mapped_addr == NULL) return NULL;
  
  numfeats = *((int*)current_sought_addr);
  current_sought_addr += sizeof(int);

  //printf("Number of feats: %d\n",numfeats);
  feats = current_sought_addr; 
  current_sought_addr += (numfeats+1)*sizeof(char*);

  for(i=0;i<numfeats;i++)
    {
      feats[i] = padded_read_string();
    }
  feats[i] = 0;
  
  return feats;
}

cst_cart* mapreader_read_tree()
{
  cst_cart* tree;
  cst_cart_node** n;
  char *** s;
  int temp;

  tree = mapreader_read_padded(&temp);
  n = &(tree->rule_table);
  s = &(tree->feat_table);

  *n = mapreader_read_tree_nodes(&temp);
  *s = mapreader_read_tree_feats(&temp);
  return tree;
}

cst_cart** mapreader_read_tree_array()
{
  cst_cart** trees;
  int numtrees;
  int i;

  if(voice_mapped_addr == NULL) return NULL;

  numtrees = *((int*)current_sought_addr);
  current_sought_addr += sizeof(int*);

  //printf("Number of trees: %d\n",numtrees);
  trees = current_sought_addr; 
  current_sought_addr += (numtrees+1)*sizeof(cst_cart*);

  for(i=0;i<numtrees;i++)
    {
      //printf("tree %d\n",i);
      trees[i] = mapreader_read_tree();
    }
  trees[i] = 0;

  return trees; 
}

void* mapreader_read_array()
{
  int temp;
  void* ret;
  ret = mapreader_read_padded(&temp);
  
  //printf("Reading array of %d bytes with first position %d\n",temp, ret);
  return ret;
}

void** mapreader_read_2d_array()
{
  int numrows;
  int i;
  
  void** arrayrows;
  
  numrows = *((int*)current_sought_addr);
  current_sought_addr += sizeof(int*);

  //printf("%d rows\n",numrows);

  arrayrows = current_sought_addr;
  current_sought_addr += sizeof(void*) * numrows;

  for(i=0;i<numrows;i++)
    arrayrows[i] = mapreader_read_array();

  return arrayrows; 
  
}

dur_stat** mapreader_read_dur_stats()
{
  int numstats;
  int i,temp;
  dur_stat** ds;

  numstats = *((int*)current_sought_addr);
  current_sought_addr += sizeof(int*);

  //printf("%d durstats\n",numstats);
  
  ds = current_sought_addr;


  current_sought_addr+= (1+numstats)*sizeof(dur_stat*);
  // load structuer values
  for(i=0;i<numstats;i++)
    ds[i] = mapreader_read_padded(&temp);
  ds[i] = NULL;

  // load string resources
  for(i=0;i<numstats;i++)
    ds[i]->phone = mapreader_read_padded(&temp);

  return ds;
}

char*** mapreader_read_phone_states()
{
  int i,j,count1,count2,temp;
  char*** ps;

  count1 = *((int*)current_sought_addr);
  current_sought_addr += sizeof(int*);

  ps = mapreader_read_padded(&temp);
  //printf("count1: %d\n",count1);
  for(i=0;i<count1;i++)
    {
      count2 = *((int*)current_sought_addr);
      current_sought_addr += sizeof(int*);
      ps[i] = mapreader_read_padded(&temp);
      //printf("   count2: %d\n",count2);
      for(j=0;j<count2;j++)
	{
	  ps[i][j]=mapreader_read_padded(&temp);
	  //printf("     %s\n",ps[i][j]);
	}
      ps[i][j] = 0;
    }
  ps[i] = 0;

  return ps;
}

void mapreader_read_voice_feature(char** fname, char** fval)
{
  int temp;
  *fname = mapreader_read_padded(&temp);
  *fval = mapreader_read_padded(&temp);
}

int mapreader_read_int()
{
  int val;
  val = *((int*)current_sought_addr);
  current_sought_addr += sizeof(int*);
  return val;
}
