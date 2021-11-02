#include <lazrs.h>
#include <las.h>
#include <point.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <iostream>

void par_decompress(char* fpath, las_header header, const las_vlr *laszip_vlr)
{
  FILE *file = fopen(fpath, "rb");
  if (file == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  fseek(file, header.offset_to_point_data, SEEK_SET);

  ParLasZipDecompressor *decompressor = NULL;
  LazrsResult result = lazrs_pardecompressor_new_file(
      &decompressor,
      file,
      laszip_vlr->data,
      laszip_vlr->record_len);
  if (result != LazrsResult::LAZRS_OK) {
    fprintf(stderr, "Failed to create the decompressor");
    exit(EXIT_FAILURE);
  }

  size_t num_bytes = header.point_size * header.point_count * sizeof(uint8_t);
  uint8_t *point_data = (uint8_t *)malloc(num_bytes);
  if (point_data == NULL) {
    fprintf(stderr, "OOM\n");
    exit(EXIT_FAILURE);
  }

  lazrs_pardecompress_many(decompressor, point_data, num_bytes);

  // print first 5 x,y,z
  uint8_t buf[255];
  for (int i = 0; i < 25; i++)
  {    
    memcpy(buf, point_data + (i * header.point_size), header.point_size);
    Point p = UnpackPoint(buf, header.point_format, header.point_size);
    printf("X: %d, Y: %d, Z: %d\r\n", p.x_, p.y_, p.z_);
  }
  printf("Decompressed %llu points\n", header.point_count);
  lazrs_delete_pardecompressor(decompressor);
}

void decompress(char* fpath, las_header header, const las_vlr *laszip_vlr)
{
  FILE *file = fopen(fpath, "rb");
  if (file == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  fseek(file, header.offset_to_point_data, SEEK_SET);

  LasZipDecompressor *decompressor = NULL;
  LazrsResult result = lazrs_decompressor_new_file(
      &decompressor,
      file,
      laszip_vlr->data,
      laszip_vlr->record_len);
  if (result != LazrsResult::LAZRS_OK) {
    fprintf(stderr, "Failed to create the decompressor");
    exit(EXIT_FAILURE);
  }

  size_t num_bytes = header.point_size * header.point_count * sizeof(uint8_t);
  uint8_t *point_data = (uint8_t *)malloc(num_bytes);
  if (point_data == NULL) {
    fprintf(stderr, "OOM\n");
    exit(EXIT_FAILURE);
  }

  lazrs_decompress_many(decompressor, point_data, num_bytes);

  // print first 5 x,y,z
  uint8_t buf[255];
  for (int i = 0; i < 25; i++)
  {
    memcpy(buf, point_data + (i * header.point_size), header.point_size);
    Point p = UnpackPoint(buf, header.point_format, header.point_size);
    printf("X: %d, Y: %d, Z: %d\r\n", p.x_, p.y_, p.z_);
  }
  printf("Decompressed %llu points\n", header.point_count);
  lazrs_delete_decompressor(decompressor);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s file.laz\n", argv[0]);
    return EXIT_FAILURE;
  }

  FILE *file = fopen(argv[1], "rb");
  if (file == NULL) {
    perror("fopen");
    return EXIT_FAILURE;
  }

  las_header header;
  las_error error;

  error = fread_las_header(file, &header);
  if (error != las_error_ok) {
    fprintf(stderr, "Error reading header\n");
    fclose(file);
    return EXIT_FAILURE;
  }
  printf("Version: %d.%d\n", (int)header.version_major,
         (int)header.version_minor);
  printf("Point size: %d, point count: %llu\n", header.point_size,
         header.point_count);
  print_vlrs(&header);

  const las_vlr *laszip_vlr = find_laszip_vlr(&header);
  if (laszip_vlr == NULL) {
    fprintf(stderr, "No laszip vlr found\n");
    fclose(file);
    las_clean_header(&header);
    return EXIT_FAILURE;
  }
  std::cout << std::endl;
  fclose(file);

  {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    par_decompress(argv[1], header, laszip_vlr);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Parallel decompression done in: " << (std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) /1000000.0 << "[s]" << std::endl;
  }

  std::cout << std::endl;
  
  {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    decompress(argv[1], header, laszip_vlr);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Single-thread decompression done in: " << (std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) /1000000.0 << "[s]" << std::endl;
  }

  las_clean_header(&header);
  return EXIT_SUCCESS;
}
