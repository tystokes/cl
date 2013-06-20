#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
__constant char hw[] = "Hello World\n";

__kernel void hello(__global char * out) {
  size_t tid = get_global_id(0);
  out[tid] = hw[tid];
}

__private int merge(__global const int *arr1, int arr1size, __global const int *arr2, int arr2size, __global int *out) {
  
  int x = 0;
  int y = 0;
  int n = 0;

  while(x < arr1size || y < arr2size) {
    if(x < arr1size && y < arr2size) {
      if(arr1[x] < arr2[y]) {
        out[n] = arr1[x];
        n++,x++;
      } else {
        out[n] = arr2[y];
        n++,y++;
      }
    } else if(x < arr1size) {
      out[n] = arr1[x];
      n++,x++;
    } else if(y < arr2size) {
      out[n] = arr2[y];
      n++,y++;
    }
  }
  return (x + y) - (arr1size + arr2size); 
}

__private int cpy(__global int *out, __global const int *in, int size) {
  int x = 0;
  for(x = 0; x < size; x++) {
    out[x] = in[x];
  }
  return 1;
}

__kernel void mergesort(__global int *a, __global int *b, __global int *len) {
  int size = 1;
  int indx = get_global_id(0) * 2;
  int i = 0;

  while(size <= *len) {
    barrier(CLK_LOCAL_MEM_FENCE);
    if((indx + size) < *len) {
      i = (indx + size) + (((indx + (size * 2)) > *len) ? *len - (indx + size) : size);
      merge(&a[indx], size, &a[indx + size], ((indx + (size * 2)) > *len) ? *len - (indx + size) : size, &b[indx]);
    } else {
      return;
    }
    size *= 2;
    cpy(&a[indx], &b[indx], size);
    if(indx%(2*size) != 0) {
      return;
    }
  }
  *len = i;
}
