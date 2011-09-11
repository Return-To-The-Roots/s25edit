#include "CFile.h"

FILE *CFile::fp = NULL;
bobBMP *CFile::bmpArray = global::bmpArray;
bobSHADOW *CFile::shadowArray = global::shadowArray;
bobPAL *CFile::palArray = global::palArray;
bobPAL *CFile::palActual = global::palArray;
bool CFile::loadPAL = false;

CFile::CFile()
{
}

CFile::~CFile()
{
}

void* CFile::open_file(char *filename, char filetype, bool only_loadPAL)
{
    void *return_value = (int*)(-1);

    if ( filename == NULL || bmpArray == NULL || shadowArray == NULL || palArray == NULL || palActual == NULL )
        return NULL;

    else if ( (fp = fopen(filename, "rb")) == NULL )
        return NULL;

    if (only_loadPAL)
        loadPAL = true;

    switch (filetype)
    {
        case LST:   if ( open_lst() == false )
                        return_value = NULL;

                    break;

        case BOB:   if ( open_bob() == false )
                        return_value = NULL;

                    break;

        case IDX:   if ( open_idx(filename) == false )
                        return_value = NULL;

                    break;

        case BBM:   if ( open_bbm() == false )
                        return_value = NULL;

                    break;

        case LBM:   if ( open_lbm(filename) == false )
                        return_value = NULL;

                    break;

        case GOU:   if ( open_gou() == false )
                        return_value = NULL;

                    break;

        case WLD:   return_value = open_wld();
                    break;

        case SWD:   return_value = open_swd();
                    break;

        default:    //no valid data type
                    return_value = NULL;

                    break;
    }

    if (fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }

    loadPAL = false;

    return return_value;
}

void* CFile::open_file(const char *filename, char filetype, bool only_loadPAL)
{
    char *file = (char*)filename;
    return open_file(file, filetype, only_loadPAL);
}

bool CFile::open_lst(void)
{
    //type of entry (used or unused entry)
    Uint16 entrytype;
    //bobtype of the entry
    Uint16 bobtype;


    //skip: id (2x 1 Bytes) + count (1x 4 Bytes) = 6 Bytes
    fseek(fp, 6, SEEK_SET);

    //main loop for reading entrys
    while(!feof(fp))
    {
        //entry type (2 Bytes) - unused (=0x0000) or used (=0x0001) -
        fread(&entrytype, 2, 1, fp);

        //if entry is unused, go back to 'while' --- and by the way: after the last entry there are always zeros in the file,
        //so the following case will happen till we have reached the end of the file and the 'while' will break - PERFECT!
        if ( entrytype == 0x0000 )
            continue;

        //bobtype (2 Bytes)
        fread(&bobtype, 2, 1, fp);

        switch (bobtype)
        {
            case BOBTYPE01:     if ( read_bob01() == false )
                                    return false;
                                break;

            case BOBTYPE02:     if ( read_bob02() == false )
                                    return false;
                                break;

            case BOBTYPE03:     if ( read_bob03() == false )
                                    return false;
                                break;

            case BOBTYPE04:     if ( read_bob04(PLAYER_BLUE) == false )
                                    return false;
                                break;

            case BOBTYPE05:     if ( read_bob05() == false )
                                    return false;
                                break;

            case BOBTYPE07:     if ( read_bob07() == false )
                                    return false;
                                break;

            case BOBTYPE14:     if ( read_bob14() == false )
                                    return false;
                                break;

            default:            //Something is wrong? Maybe the last entry was really the LAST, so we should not return false
                                break;
        }

    }

    return true;

}

bool CFile::open_bob(void)
{
    return false;
}

bool CFile::open_idx(char *filename)
{
    //temporary filepointer to save global fp until this function has finished
    FILE *fp_tmp;
    //pointer to '******.IDX'-File (cause we have to change the global pointer to point to the '.DAT'-File)
    FILE *fp_idx;
    //pointer to corresponding '******.DAT'-File
    FILE *fp_dat;
    //name of corresponging '******.DAT'-File
    char *filename_dat;
    //array index for the first letter of the file ending ( the 'I' in 'IDX' ) to overwrite IDX with DAT
    int fileending;
    //starting adress of data in the corresponding '******.DAT'-File
    Uint32 offset;
    //bobtype of the entry
    Uint16 bobtype;
    //bobtype is repeated in the corresponging '******.DAT'-File, so we have to check this is correct
    Uint16 bobtype_check;

    //save global filepointer
    fp_tmp = fp;
    //get a new filepointer to the '.IDX'-File
    if ( (fp_idx = fopen(filename, "rb")) == NULL)
        return false;
    //following code will open the corresponding '******.DAT'-File
    //allocate memory for new name
    if ( (filename_dat = (char*) malloc( strlen(filename) + 1 ) ) == NULL )
        return false;
    //fill new allocated memory with name of 'IDX'-File
    strcpy(filename_dat, filename);
    //if strlen = n, so array walks von 0 to n-1. n-1 is the last letter of the file ending ( the 'X' in 'IDX' ), so we have to walk back
    //one time to be at the last letter and then two times to be at the 'I' = walk back 3 times
    fileending = strlen(filename) - 3;
    //now overwrite 'IDX' with 'DAT'
    filename_dat[fileending]     = 'D';
    filename_dat[fileending + 1] = 'A';
    filename_dat[fileending + 2] = 'T';
    //get the filepointer of the corresponging '******.DAT'-File
    if ( (fp_dat = fopen(filename_dat, "rb")) == NULL)
        return false;
    //we are finished opening the 'DAT'-File, now we can handle the content


    //skip: unknown data (1x 4 Bytes) at the beginning of the file
    fseek(fp_idx, 4, SEEK_SET);

    //main loop for reading entrys
    while( !feof(fp_idx) && !feof(fp_dat) )
    {
        //skip: name (1x 16 Bytes)
        fseek(fp_idx, 16, SEEK_CUR);
        //offset (4 Bytes)
        fread(&offset, 4, 1, fp_idx);
        //skip unknown data (6x 1 Byte)
        fseek(fp_idx, 6, SEEK_CUR);
        //bobtype (2 Bytes)
        fread(&bobtype, 2, 1, fp_idx);
        //set fp_dat to the position in 'offset'
        fseek(fp_dat, offset, SEEK_SET);
        //read bobtype again, now from 'DAT'-File
        fread(&bobtype_check, 2, 1, fp_dat);
        //check if data in 'DAT'-File is the data that it should be (bobtypes are equal)
        if (bobtype != bobtype_check)
            return false;


        //set up the global filepointer fp to fp_dat, so the following functions will read from the '.DAT'-File
        fp = fp_dat;

        switch (bobtype)
        {
            case BOBTYPE01:     if ( read_bob01() == false )
                                    return false;
                                break;

            case BOBTYPE02:     if ( read_bob02() == false )
                                    return false;
                                break;

            case BOBTYPE03:     if ( read_bob03() == false )
                                    return false;
                                break;

            case BOBTYPE04:     if ( read_bob04() == false )
                                    return false;
                                break;

            case BOBTYPE05:     if ( read_bob05() == false )
                                    return false;
                                break;

            case BOBTYPE07:     if ( read_bob07() == false )
                                    return false;
                                break;

            case BOBTYPE14:     if ( read_bob14() == false )
                                    return false;
                                break;

            default:            //Something is wrong? Maybe the last entry was really the LAST, so we should not return false
                                break;
        }

        //set up the local filepointer fp_dat to fp for the next while loop
        fp_dat = fp;
    }

    //reset global filepointer to its original value
    fp = fp_tmp;

    fclose(fp_idx);
    fclose(fp_dat);
    free(filename_dat);

    return true;
}

bool CFile::open_bbm(void)
{
    //skip header (48 Bytes)
    fseek(fp, 48, SEEK_CUR);

    for (int i = 0; i < 256; i++)
    {
        fread(&((palArray->colors[i]).r), 1, 1, fp);
        fread(&((palArray->colors[i]).g), 1, 1, fp);
        fread(&((palArray->colors[i]).b), 1, 1, fp);
    }

    palArray++;

    return true;
}

bool CFile::open_lbm(char *filename)
{
    //IMPORTANT NOTE: LBM (also ILBM) is the Interchange File Format (IFF) and the 4-byte-blocks are originally organized as Big Endian, so a convertion is needed

    //identifier for the kind of data follows (FORM, BMHD, CMAP, BODY)
    char chunk_identifier[5];

    //length of data block
    Uint32 length;
    //color depth of the picture
    Uint16 color_depth;
    //is the picture rle-compressed?
    Uint16 compression_flag;
    //compression type
    Sint8 ctype;
    //array for palette colors
    SDL_Color colors[256];
    //color value for read pixel
    Uint8 color_value;


    //skip File-Identifier "FORM" (1x 4 Bytes) + unknown data (4x 1 Byte) + Header-Identifier "PBM " (1x 4 Bytes) = 12 Bytes
    fseek(fp, 12, SEEK_CUR);

    /* READ FIRST CHUNK "BMHD" */

    //chunk-identifier (4 Bytes)
    fread(chunk_identifier, 4, 1, fp);
    chunk_identifier[4] = '\0';
    //should be "BMHD" at this time
    if (strcmp(chunk_identifier, "BMHD") != 0)
        return false;
    //length of data block
    fread(&length, 4, 1, fp);
    endian_swap(length);
    //width of picture (2 Bytes)
    fread(&(bmpArray->w), 2, 1, fp);
    endian_swap(bmpArray->w);
    //heigth of picture (2 Bytes)
    fread(&(bmpArray->h), 2, 1, fp);
    endian_swap(bmpArray->h);
    //skip unknown data (4x 1 Bytes)
    fseek(fp, 4, SEEK_CUR);
    //color depth of the picture (1x 2 Bytes)
    fread(&color_depth, 2, 1, fp);
    //compression_flag (1x 2 Bytes)
    fread(&compression_flag, 2, 1, fp);
    //skip unknown data (length - 20 x 1 Byte)
    //fseek(fp, length-20, SEEK_CUR);

    /* READ SECOND CHUNK "CMAP" */

    //chunk-identifier (4 Bytes)
    //search for the "CMAP" and skip other chunk-types
    while (!feof(fp))
    {
        fread(chunk_identifier, 4, 1, fp);

        if (strcmp(chunk_identifier, "CMAP") == 0)
            break;
        else
            fseek(fp, -3, SEEK_CUR);
    }
    if (feof(fp))
        return false;
    //length of data block
    fread(&length, 4, 1, fp);
    endian_swap(length);
    //must be 768 (RGB = 3 Byte x 256 Colors)
    if (length != 768)
        return false;
    //palette
    for (int i = 0; i < 256; i++)
    {
        fread(&colors[i].r, 1, 1, fp);
        fread(&colors[i].g, 1, 1, fp);
        fread(&colors[i].b, 1, 1, fp);
    }

    /* READ THIRD CHUNK "BODY" */

    //chunk-identifier (4 Bytes)
    //search for the "BODY" and skip other chunk-types
    while (!feof(fp))
    {
        fread(chunk_identifier, 4, 1, fp);

        if (strcmp(chunk_identifier, "BODY") == 0)
            break;
        else
            fseek(fp, -3, SEEK_CUR);
    }
    if (feof(fp))
        return false;
    //length of data block
    fread(&length, 4, 1, fp);
    endian_swap(length);


    //now we are ready to read the picture lines and fill the surface, so lets create one
    if ( (bmpArray->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, bmpArray->w, bmpArray->h, 8, 0, 0, 0, 0)) == NULL )
        return false;
    SDL_SetPalette(bmpArray->surface, SDL_LOGPAL, colors, 0, 256);

    if (compression_flag == 0)
    {
        //picture uncompressed
        //main loop for reading picture lines
        for (int y = 0; y < bmpArray->h; y++)
        {
            //loop for reading pixels of the actual picture line
            for (int x = 0; x < bmpArray->w; x++)
            {
                //read color value (1 Byte)
                fread(&color_value, 1, 1, fp);
                //draw
                CSurface::DrawPixel_Color(bmpArray->surface, x, y, (Uint32)color_value);
            }
        }

    }
    else if (compression_flag == 1)
    {
        //picture compressed

        //main loop for reading picture lines
        for (int y = 0; y < bmpArray->h; y++)
        {
            //loop for reading pixels of the actual picture line
            //(cause of a kind of RLE-Compression we cannot read the pixels sequentielly)
            //'x' will be incremented WITHIN the loop
            for (int x = 0; x < bmpArray->w; )
            {
                //read compression type
                fread(&ctype, 1, 1, fp);

                if (ctype >= 0)
                {
                    //following 'ctype + 1' pixels are uncompressed
                    for (int k = 0; k < ctype + 1; k++, x++)
                    {
                        //read color value (1 Byte)
                        fread(&color_value, 1, 1, fp);
                        //draw
                        CSurface::DrawPixel_Color(bmpArray->surface, x, y, (Uint32)color_value);
                    }
                }
                else if (ctype < 0 && ctype >= -127)
                {
                    //draw the following byte '-ctype + 1' times to the surface
                    fread(&color_value, 1, 1, fp);

                    for (int k = 0; k < -ctype + 1; k++, x++)
                        CSurface::DrawPixel_Color(bmpArray->surface, x, y, (Uint32)color_value);
                }
                else //if (ctype == -128)
                {
                    //ignore
                }
            }
        }
    }
    else
        return false;

    //if this is a texture file, we need a secondary 32-bit surface for SGE and we set a color key at both surfaces
    if (    strcmp(filename, "./GFX/TEXTURES/TEX5.LBM") == 0
         || strcmp(filename, "./GFX/TEXTURES/TEX6.LBM") == 0
         || strcmp(filename, "./GFX/TEXTURES/TEX7.LBM") == 0
         || strcmp(filename, "./GFX/TEXTURES/TEXTUR_0.LBM") == 0
         || strcmp(filename, "./GFX/TEXTURES/TEXTUR_3.LBM") == 0
       )
    {
        SDL_SetColorKey(bmpArray->surface, SDL_SRCCOLORKEY, SDL_MapRGB(bmpArray->surface->format, 0, 0, 0));

        bmpArray++;
        if ( (bmpArray->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, (bmpArray-1)->w, (bmpArray-1)->h, 32, 0, 0, 0, 0)) != NULL )
        {
            SDL_SetColorKey(bmpArray->surface, SDL_SRCCOLORKEY, SDL_MapRGB(bmpArray->surface->format, 0, 0, 0));
            CSurface::Draw(bmpArray->surface, (bmpArray-1)->surface, 0, 0);
        }
        else
            bmpArray--;
    }

    //we are finished, the surface is filled
    //increment bmpArray for the next picture
    bmpArray++;

    return true;
}

bool CFile::open_gou(void)
{
    static int internalArrayCtr = 0; //maximum is two cause there are only 3 GOUx.DAT-Files

    if (internalArrayCtr > 2)
        return false;

    fread(gouData[internalArrayCtr], 256, 256, fp);
    internalArrayCtr++;

    return true;
}

bobMAP* CFile::open_wld(void)
{
    bobMAP *myMap = (bobMAP*)malloc(sizeof(bobMAP));
    Uint8 heightFactor;

    if (myMap == NULL)
        return myMap;

    //initialize myMap->name and myMap->author (both 20 chars) to prevent filling with random memory content
    for (int i = 0; i < 20; i++)
    {
        myMap->name[i] = '\0';
        myMap->author[i] = '\0';
    }

    fseek(fp, 10, SEEK_SET);
    fread(myMap->name, 20, 1, fp);
    fread(&myMap->width_old, 2, 1, fp);
    fread(&myMap->height_old, 2, 1, fp);
    fread(&myMap->type, 1, 1, fp);
    fread(&myMap->player, 1, 1, fp);
    fread(myMap->author, 20, 1, fp);
    fread(myMap->HQx, 2, 7, fp);
    fread(myMap->HQy, 2, 7, fp);

    //go to big map header and read it
    fseek(fp, 92, SEEK_SET);
    for (int i = 0; i < 250; i++)
    {
        fread(&myMap->header[i].type, 1, 1, fp);
        fread(&myMap->header[i].x, 1, 2, fp);
        fread(&myMap->header[i].y, 1, 2, fp);
        fread(&myMap->header[i].area, 1, 4, fp);
    }

    //go to real map height and width
    fseek(fp, 2348, SEEK_SET);
    fread(&myMap->width, 2, 1, fp);
    myMap->width_pixel = myMap->width*TRIANGLE_WIDTH;
    fread(&myMap->height, 2, 1, fp);
    myMap->height_pixel = myMap->height*TRIANGLE_HEIGHT;

    if ( (myMap->vertex = (struct point*) malloc(sizeof(struct point)*myMap->width*myMap->height)) == NULL )
    {
        free(myMap);
        return NULL;
    }

    //go to altitude information (we skip the 16 bytes long map data header that each block has)
    fseek(fp, 16, SEEK_CUR);

    int a;
    int b = 0;
    for (int j = 0; j < myMap->height; j++)
    {
        if (j%2 == 0)
            a = TRIANGLE_WIDTH/2;
        else
            a = TRIANGLE_WIDTH;

        for (int i = 0; i < myMap->width; i++)
        {
            myMap->vertex[j*myMap->width+i].VertexX = i;
            myMap->vertex[j*myMap->width+i].VertexY = j;
            fread(&heightFactor, 1, 1, fp);
            myMap->vertex[j*myMap->width+i].h = heightFactor;
            myMap->vertex[j*myMap->width+i].x = a;
            myMap->vertex[j*myMap->width+i].y = b + (-TRIANGLE_INCREASE)*(heightFactor - 0x0A);
            myMap->vertex[j*myMap->width+i].z = TRIANGLE_INCREASE*(heightFactor - 0x0A);
            //TEMPORARY: to prevent drawing point outside the surface (negative points)
            //if (myMap->vertex[j*myMap->width+i].y < 0)
                //myMap->vertex[j*myMap->width+i].y = 0;
            a += TRIANGLE_WIDTH;
        }
        b += TRIANGLE_HEIGHT;
    }

    //go to texture information for RightSideUp-Triangles
    fseek(fp, 16, SEEK_CUR);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fread(&myMap->vertex[j*myMap->width+i].rsuTexture, 1, 1, fp);
    }

    //go to texture information for UpSideDown-Triangles
    fseek(fp, 16, SEEK_CUR);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fread(&myMap->vertex[j*myMap->width+i].usdTexture, 1, 1, fp);
    }

    //go to road data
    fseek(fp, 16, SEEK_CUR);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fread(&myMap->vertex[j*myMap->width+i].road, 1, 1, fp);
    }

    //go to object type data
    fseek(fp, 16, SEEK_CUR);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fread(&myMap->vertex[j*myMap->width+i].objectType, 1, 1, fp);
    }

    //go to object info data
    fseek(fp, 16, SEEK_CUR);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fread(&myMap->vertex[j*myMap->width+i].objectInfo, 1, 1, fp);
    }

    //go to animal data
    fseek(fp, 16, SEEK_CUR);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fread(&myMap->vertex[j*myMap->width+i].animal, 1, 1, fp);
    }

    //go to unknown1 data
    fseek(fp, 16, SEEK_CUR);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fread(&myMap->vertex[j*myMap->width+i].unknown1, 1, 1, fp);
    }

    //go to build data
    fseek(fp, 16, SEEK_CUR);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fread(&myMap->vertex[j*myMap->width+i].build, 1, 1, fp);
    }

    //go to unknown2 data
    fseek(fp, 16, SEEK_CUR);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fread(&myMap->vertex[j*myMap->width+i].unknown2, 1, 1, fp);
    }

    //go to unknown3 data
    fseek(fp, 16, SEEK_CUR);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fread(&myMap->vertex[j*myMap->width+i].unknown3, 1, 1, fp);
    }

    //go to resource data
    fseek(fp, 16, SEEK_CUR);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fread(&myMap->vertex[j*myMap->width+i].resource, 1, 1, fp);
    }

    //go to shading data
    fseek(fp, 16, SEEK_CUR);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fread(&myMap->vertex[j*myMap->width+i].shading, 1, 1, fp);
    }

    //go to unknown5 data
    fseek(fp, 16, SEEK_CUR);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fread(&myMap->vertex[j*myMap->width+i].unknown5, 1, 1, fp);
    }

    return myMap;
}

bobMAP* CFile::open_swd(void)
{
    return open_wld();
}

bool CFile::save_file(char *filename, char filetype, void *data)
{
    bool return_value = false;

    if ( filename == NULL || data == NULL )
        return return_value;

    if ( (fp = fopen(filename, "wb")) == NULL )
        return return_value;

    switch (filetype)
    {
        case LST:   return_value = save_lst(data);
                    break;

        case BOB:   return_value = save_bob(data);
                    break;

        case IDX:   return_value = save_idx(data, filename);
                    break;

        case BBM:   return_value = save_bbm(data);
                    break;

        case LBM:   return_value = save_lbm(data);
                    break;

        case WLD:   return_value = save_wld(data);
                    break;

        case SWD:   return_value = save_swd(data);
                    break;

        default:    //no valid data type
                    return_value = false;
                    break;
    }

    if (fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }

    return return_value;
}

bool CFile::save_file(const char *filename, char filetype, void *data)
{
    char *file = (char*)filename;
    return save_file(file, filetype, data);
}

bool CFile::save_lst(void *data)
{
    return true;
}

bool CFile::save_bob(void *data)
{
    return true;
}

bool CFile::save_idx(void *data, char *filename)
{
    return true;
}

bool CFile::save_bbm(void *data)
{
    return true;
}

bool CFile::save_lbm(void *data)
{
    return true;
}

bool CFile::save_wld(void *data)
{
    char zero = 0; //to fill bytes
    char temp = 0; //to fill bytes
    bobMAP *myMap = (bobMAP*)data;
    char map_version[11] = "WORLD_V1.0";
    char map_data_header[16];

    //prepare map data header
    map_data_header[0] = 0x10;
    map_data_header[1] = 0x27;
    map_data_header[2] = 0x00;
    map_data_header[3] = 0x00;
    map_data_header[4] = 0x00;
    map_data_header[5] = 0x00;
    *((Uint16*)(map_data_header+6)) = myMap->width;
    *((Uint16*)(map_data_header+8)) = myMap->height;
    map_data_header[10] = 0x01;
    map_data_header[11] = 0x00;
    *((Uint32*)(map_data_header+12)) = myMap->width*myMap->height;

    //begin writing data to file
    //first of all the map header
    //WORLD_V1.0
    fwrite(map_version, 1, 10, fp);
    //name
    fwrite(myMap->name, 1, 20, fp);
    //old width
    fwrite(&myMap->width_old, 2, 1, fp);
    //old height
    fwrite(&myMap->height_old, 2, 1, fp);
    //type
    fwrite(&myMap->type, 1, 1, fp);
    //players
    fwrite(&myMap->player, 1, 1, fp);
    //author
    fwrite(myMap->author, 1, 20, fp);
    //headquarters x
    fwrite(myMap->HQx, 2, 7, fp);
    //headquarters y
    fwrite(myMap->HQy, 2, 7, fp);
    //unknown data (8 Bytes)
    for (int i = 0; i < 8; i++)
        fwrite(&zero, 1, 1, fp);
    //big map header with area information
    for (int i = 0; i < 250; i++)
    {
        fwrite(&myMap->header[i].type, 1, 1, fp);
        fwrite(&myMap->header[i].x, 1, 2, fp);
        fwrite(&myMap->header[i].y, 1, 2, fp);
        fwrite(&myMap->header[i].area, 1, 4, fp);
    }
    //0x11 0x27
    temp = 0x11;
    fwrite(&temp, 1, 1, fp);
    temp = 0x27;
    fwrite(&temp, 1, 1, fp);
    //unknown data (always null, 4 Bytes)
    for (int i = 0; i < 4; i++)
        fwrite(&zero, 1, 1, fp);
    //width
    fwrite(&myMap->width, 2, 1, fp);
    //height
    fwrite(&myMap->height, 2, 1, fp);


    //now begin writing the real map data

    //altitude information
    fwrite(&map_data_header, 16, 1, fp);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
        {
            temp = myMap->vertex[j*myMap->width+i].z/5 + 0x0A;
            fwrite(&temp, 1, 1, fp);
        }
    }

    //texture information for RightSideUp-Triangles
    fwrite(&map_data_header, 16, 1, fp);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fwrite(&myMap->vertex[j*myMap->width+i].rsuTexture, 1, 1, fp);
    }

    //go to texture information for UpSideDown-Triangles
    fwrite(&map_data_header, 16, 1, fp);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fwrite(&myMap->vertex[j*myMap->width+i].usdTexture, 1, 1, fp);
    }

    //go to road data
    fwrite(&map_data_header, 16, 1, fp);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fwrite(&myMap->vertex[j*myMap->width+i].road, 1, 1, fp);
    }

    //go to object type data
    fwrite(&map_data_header, 16, 1, fp);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fwrite(&myMap->vertex[j*myMap->width+i].objectType, 1, 1, fp);
    }

    //go to object info data
    fwrite(&map_data_header, 16, 1, fp);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fwrite(&myMap->vertex[j*myMap->width+i].objectInfo, 1, 1, fp);
    }

    //go to animal data
    fwrite(&map_data_header, 16, 1, fp);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fwrite(&myMap->vertex[j*myMap->width+i].animal, 1, 1, fp);
    }

    //go to unknown1 data
    fwrite(&map_data_header, 16, 1, fp);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fwrite(&myMap->vertex[j*myMap->width+i].unknown1, 1, 1, fp);
    }

    //go to build data
    fwrite(&map_data_header, 16, 1, fp);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fwrite(&myMap->vertex[j*myMap->width+i].build, 1, 1, fp);
    }

    //go to unknown2 data
    fwrite(&map_data_header, 16, 1, fp);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fwrite(&myMap->vertex[j*myMap->width+i].unknown2, 1, 1, fp);
    }

    //go to unknown3 data
    fwrite(&map_data_header, 16, 1, fp);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fwrite(&myMap->vertex[j*myMap->width+i].unknown3, 1, 1, fp);
    }

    //go to resource data
    fwrite(&map_data_header, 16, 1, fp);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fwrite(&myMap->vertex[j*myMap->width+i].resource, 1, 1, fp);
    }

    //go to shading data
    fwrite(&map_data_header, 16, 1, fp);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fwrite(&myMap->vertex[j*myMap->width+i].shading, 1, 1, fp);
    }

    //go to unknown5 data
    fwrite(&map_data_header, 16, 1, fp);

    for (int j = 0; j < myMap->height; j++)
    {
        for (int i = 0; i < myMap->width; i++)
            fwrite(&myMap->vertex[j*myMap->width+i].unknown5, 1, 1, fp);
    }

    //at least write the map footer (ends in 0xFF)
    temp = 0xFF;
    fwrite(&temp, 1, 1, fp);

    return true;
}

bool CFile::save_swd(void *data)
{
    return save_wld(data);
}


bool CFile::read_bob01(void)
{
    return true;
}

bool CFile::read_bob02(void)
{
    //length of data block
    Uint32 length;
    //offset of next entry in file
    Uint32 next_entry;
    //endmark - to test, if a data block has correctly read
    Uint8 endmark;

    //Pic-Data
    //start adresses of picture lines in the file (offsets)
    Uint16 *starts;
    //starting point for start adresses (starting_point + starts[i] = beginning of i. picture line)
    Uint32 starting_point;
    //number of following colored pixels in data block
    Uint8 count_color;
    //number of transparent pixels
    Uint8 count_trans;
    //color value for read pixel
    Uint8 color_value;


    //coordinate for zeropoint x (2 Bytes)
    fread(&(bmpArray->nx), 2, 1, fp);
    //coordinate for zeropoint y (2 Bytes)
    fread(&(bmpArray->ny), 2, 1, fp);
    //skip unknown data (4x 1 Byte)
    fseek(fp, 4, SEEK_CUR);
    //width of picture (2 Bytes)
    fread(&(bmpArray->w), 2, 1, fp);
    //heigth of picture (2 Bytes)
    fread(&(bmpArray->h), 2, 1, fp);
    //skip unknown data (1x 2 Bytes)
    fseek(fp, 2, SEEK_CUR);
    //length of datablock (1x 4 Bytes)
    fread(&length, 4, 1, fp);
    //fp points now ON the first start adress, so "actual position + length = first offset of next entry in the file"
    starting_point = ftell(fp);
    next_entry = starting_point + length;

    //if we only want to read a palette at the moment (loadPAL == 1) so skip this and set the filepointer to the next entry
    if (loadPAL)
    {
        fseek(fp, (long int) next_entry, SEEK_SET);
        return true;
    }

    //array for start adresses of picture lines
    if ( (starts = (Uint16*) malloc( bmpArray->h * sizeof(Uint16) )) == NULL )
        return false;

    //read start adresses
    for (int y = 0; y < bmpArray->h; y++)
        fread(&starts[y], 2, 1, fp);

    //now we are ready to read the picture lines and fill the surface, so lets create one
    if ( (bmpArray->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, bmpArray->w, bmpArray->h, 8, 0, 0, 0, 0)) == NULL )
        return false;
    SDL_SetPalette(bmpArray->surface, SDL_LOGPAL, palActual->colors, 0, 256);
    SDL_SetColorKey(bmpArray->surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(bmpArray->surface->format, 0, 0, 0));
    //SDL_SetAlpha(bmpArray->surface, SDL_SRCALPHA, 128);

    //main loop for reading picture lines
    for (int y = 0; y < bmpArray->h; y++)
    {
        //set fp to the needed offset (where the picture line starts)
        fseek(fp, starting_point + (Uint32)starts[y], SEEK_SET);

        //loop for reading pixels of the actual picture line
        //(cause of a kind of RLE-Compression we cannot read the pixels sequentielly)
        //'x' will be incremented WITHIN the loop
        for (int x = 0; x < bmpArray->w; )
        {
            //number of following colored pixels (1 Byte)
            fread(&count_color, 1, 1, fp);

            //loop for drawing the colored pixels to the surface
            for (int k = 0; k < count_color; k++, x++)
            {
                //read color value (1 Byte)
                fread(&color_value, 1, 1, fp);
                //draw
                CSurface::DrawPixel_Color(bmpArray->surface, x, y, (Uint32)color_value);
            }

            //number of transparent pixels to draw now (1 Byte)
            fread(&count_trans, 1, 1, fp);

            //loop for drawing the transparent pixels to the surface
            for (int k = 0; k < count_trans; k++, x++)
            {
                CSurface::DrawPixel_RGBA(bmpArray->surface, x, y, 0, 0, 0, 0);
            }
        }

        //the end of line should be 0xFF, otherwise an error has ocurred (1 Byte)
        fread(&endmark, 1, 1, fp);
        if (endmark != 0xFF)
            return false;
    }

    //at the end of the block (after the last line) there should be another 0xFF, otherwise an error has ocurred (1 Byte)
    fread(&endmark, 1, 1, fp);
    if (endmark != 0xFF)
        return false;

    //we are finished, the surface is filled
    //fp should now point to the first offset of the next entry, this is the last check if this is REALLY the RIGHT position
    if ( ftell(fp) != (long int) next_entry )
    {
        fseek(fp, (long int) next_entry, SEEK_SET);
        return false;
    }

    //increment bmpArray for the next picture
    bmpArray++;

    free(starts);

    return true;
}

bool CFile::read_bob03(void)
{
    //bobtype of the entry
    Uint16 bobtype;
    //player color
    int player_color;
    //save position of the filepointer to read the character again with another color
    long int offset;

    //temporary skip x- and y-spacing (2x 1 Byte) --> we will handle this later
    fseek(fp, 2, SEEK_CUR);

    //read bobtype04 115 times (for 115 ansi chars)
    for (int i = 1; i <= 115; i++)
    {
        //following data blocks are bobtype04 for some ascii chars, bobtype is repeated at the beginning of each data block
        fread(&bobtype, 2, 1, fp);

        //bobtype should be 04. if not, it's possible that there are a lot of zeros till the next block begins
        if (bobtype != BOBTYPE04)
        {
            //read the zeros (2 Bytes for each zero)
            while (bobtype == 0)
                fread(&bobtype, 2, 1, fp);

            //at the end of all the zeros --> if bobtype is STILL NOT 04, an error has occured
            if (bobtype != BOBTYPE04)
                return false;
        }

        //now read the picture for each player color
        offset = ftell(fp);
        for (int i = 0; i < 7; i++)
        {
            switch (i)
            {
                case 0: player_color = PLAYER_BLUE;
                        break;
                case 1: player_color = PLAYER_RED;
                        break;
                case 2: player_color = PLAYER_ORANGE;
                        break;
                case 3: player_color = PLAYER_GREEN;
                        break;
                case 4: player_color = PLAYER_MINTGREEN;
                        break;
                case 5: player_color = PLAYER_YELLOW;
                        break;
                case 6: player_color = PLAYER_RED_BRIGHT;
                        break;
                default:player_color = PLAYER_YELLOW;
                        break;
            }
            fseek(fp, offset, SEEK_SET);
            if ( read_bob04(player_color) == false )
                return false;
        }
    }

    return true;
}


bool CFile::read_bob04(int player_color)
{
    //length of data block
    Uint32 length;
    //offset of next entry in file
    Uint32 next_entry;

    //Pic-Data
    //'shift' (1 Byte) --> to decide what pixels and how many we have to draw
    Uint8 shift;
    //start adresses of picture lines in the file (offsets)
    Uint16 *starts;
    //starting point for start adresses (starting_point + starts[i] = beginning of i. picture line)
    Uint32 starting_point;
    //color value for read pixel
    Uint8 color_value;


    //coordinate for zeropoint x (2 Bytes)
    fread(&(bmpArray->nx), 2, 1, fp);
    //coordinate for zeropoint y (2 Bytes)
    fread(&(bmpArray->ny), 2, 1, fp);
    //skip unknown data (4x 1 Byte)
    fseek(fp, 4, SEEK_CUR);
    //width of picture (2 Bytes)
    fread(&(bmpArray->w), 2, 1, fp);
    //heigth of picture (2 Bytes)
    fread(&(bmpArray->h), 2, 1, fp);
    //skip unknown data (1x 2 Bytes)
    fseek(fp, 2, SEEK_CUR);
    //length of datablock (1x 4 Bytes)
    fread(&length, 4, 1, fp);
    //fp points now ON the first start adress, so "actual position + length = first offset of next entry in the file"
    starting_point = ftell(fp);
    next_entry = starting_point + length;

    //if we only want to read a palette at the moment (loadPAL == 1) so skip this and set the filepointer to the next entry
    if (loadPAL)
    {
        fseek(fp, (long int) next_entry, SEEK_SET);
        return true;
    }

    //array for start adresses of picture lines
    if ( (starts = (Uint16*) malloc( bmpArray->h * sizeof(Uint16) )) == NULL )
        return false;

    //read start adresses
    for (int y = 0; y < bmpArray->h; y++)
        fread(&starts[y], 2, 1, fp);

    //now we are ready to read the picture lines and fill the surface, so lets create one
    if ( (bmpArray->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, bmpArray->w, bmpArray->h, 8, 0, 0, 0, 0)) == NULL )
        return false;

    SDL_SetPalette(bmpArray->surface, SDL_LOGPAL, palActual->colors, 0, 256);
    SDL_SetColorKey(bmpArray->surface, SDL_SRCCOLORKEY, SDL_MapRGB(bmpArray->surface->format, 0, 0, 0));

    //main loop for reading picture lines
    for (int y = 0; y < bmpArray->h; y++)
    {
        //set fp to the needed offset (where the picture line starts)
        fseek(fp, starting_point + (Uint32)starts[y], SEEK_SET);

        //loop for reading pixels of the actual picture line
        //(cause of a kind of RLE-Compression we cannot read the pixels sequentielly)
        //'x' will be incremented WITHIN the loop
        for (int x = 0; x < bmpArray->w; )
        {
            //read our 'shift' (1 Byte)
            fread(&shift, 1, 1, fp);

            if (shift < 0x41)
            {
                for (int i = 1; i <= shift; i++, x++)
                {
                    CSurface::DrawPixel_RGBA(bmpArray->surface, x, y, 0, 0, 0, 0);
                }
            }
            else if (shift >= 0x41 && shift < 0x81)
            {
                fread(&color_value, 1, 1, fp);

                for (int i = 1; i <= shift - 0x40; i++, x++)
                {
                    CSurface::DrawPixel_Color(bmpArray->surface, x, y, /*0x80 - 0x40*/ + (Uint32)color_value);
                }
            }
            else if (shift >= 0x81 && shift < 0xC1)
            {
                fread(&color_value, 1, 1, fp);

                for (int i = 1; i <= shift - 0x80; i++, x++)
                {
                    CSurface::DrawPixel_Color(bmpArray->surface, x, y, player_color + (Uint32)color_value);
                }
            }
            else //if (shift > 0xC0)
            {
                fread(&color_value, 1, 1, fp);

                for (int i = 1; i <= shift - 0xC0; i++, x++)
                {
                    CSurface::DrawPixel_Color(bmpArray->surface, x, y, (Uint32)color_value);
                }
            }
        }
    }

    //we are finished, the surface is filled
    //fp should now point to the first offset of the next entry, this is the last check if this is REALLY the RIGHT position
    if ( ftell(fp) != (long int) next_entry )
    {
        fseek(fp, (long int) next_entry, SEEK_SET);
        //return false;
    }

    //increment bmpArray for the next picture
    bmpArray++;

    free(starts);

    return true;
}


bool CFile::read_bob05(void)
{
    //skip: unknown data (1x 2 Bytes)
    fseek(fp, 2, SEEK_CUR);

    for (int i = 0; i < 256; i++)
    {
        fread(&((palArray->colors[i]).r), 1, 1, fp);
        fread(&((palArray->colors[i]).g), 1, 1, fp);
        fread(&((palArray->colors[i]).b), 1, 1, fp);
    }

    palArray++;

    return true;
}


bool CFile::read_bob07(void)
{
    //length of data block
    Uint32 length;
    //offset of next entry in file
    Uint32 next_entry;
    //endmark - to test, if a data block has correctly read
    Uint8 endmark;

    //Pic-Data
    //start adresses of picture lines in the file (offsets)
    Uint16 *starts;
    //starting point for start adresses (starting_point + starts[i] = beginning of i. picture line)
    Uint32 starting_point;
    //number of half-transparent black pixels in data block
    Uint8 count_black;
    //number of transparent pixels
    Uint8 count_trans;


    //coordinate for zeropoint x (2 Bytes)
    fread(&(shadowArray->nx), 2, 1, fp);
    //coordinate for zeropoint y (2 Bytes)
    fread(&(shadowArray->ny), 2, 1, fp);
    //skip unknown data (4x 1 Byte)
    fseek(fp, 4, SEEK_CUR);
    //width of picture (2 Bytes)
    fread(&(shadowArray->w), 2, 1, fp);
    //heigth of picture (2 Bytes)
    fread(&(shadowArray->h), 2, 1, fp);
    //skip unknown data (1x 2 Bytes)
    fseek(fp, 2, SEEK_CUR);
    //length of datablock (1x 4 Bytes)
    fread(&length, 4, 1, fp);
    //fp points now ON the first start adress, so "actual position + length = first offset of next entry in the file"
    starting_point = ftell(fp);
    next_entry = starting_point + length;

    //if we only want to read a palette at the moment (loadPAL == 1) so skip this and set the filepointer to the next entry
    if (loadPAL)
    {
        fseek(fp, (long int) next_entry, SEEK_SET);
        return true;
    }

    //array for start adresses of picture lines
    if ( (starts = (Uint16*) malloc( shadowArray->h * sizeof(Uint16) )) == NULL )
        return false;

    //read start adresses
    for (int y = 0; y < shadowArray->h; y++)
        fread(&starts[y], 2, 1, fp);

    //now we are ready to read the picture lines and fill the surface, so lets create one
    if ( (shadowArray->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, shadowArray->w, shadowArray->h, 8, 0, 0, 0, 0)) == NULL )
        return false;
    SDL_SetPalette(shadowArray->surface, SDL_LOGPAL, palActual->colors, 0, 256);
    //SDL_SetAlpha(shadowArray->surface, SDL_SRCALPHA, 128);

    //main loop for reading picture lines
    for (int y = 0; y < shadowArray->h; y++)
    {
        //set fp to the needed offset (where the picture line starts)
        fseek(fp, starting_point + (Uint32)starts[y], SEEK_SET);

        //loop for reading pixels of the actual picture line
        //(cause of a kind of RLE-Compression we cannot read the pixels sequentielly)
        //'x' will be incremented WITHIN the loop
        for (int x = 0; x < shadowArray->w; )
        {
            //number of half-transparent black (alpha value = 0x40) pixels (1 Byte)
            fread(&count_black, 1, 1, fp);

            //loop for drawing the black pixels to the surface
            for (int k = 0; k < count_black; k++, x++)
            {
                //draw
                CSurface::DrawPixel_RGBA(shadowArray->surface, x, y, 0, 0, 0, 0x40);
            }

            //number of transparent pixels to draw now (1 Byte)
            fread(&count_trans, 1, 1, fp);

            //loop for drawing the transparent pixels to the surface
            for (int k = 0; k < count_trans; k++, x++)
            {
                CSurface::DrawPixel_RGBA(shadowArray->surface, x, y, 0, 0, 0, 0);
            }
        }

        //the end of line should be 0xFF, otherwise an error has ocurred (1 Byte)
        fread(&endmark, 1, 1, fp);
        if (endmark != 0xFF)
            return false;
    }

    //at the end of the block (after the last line) there should be another 0xFF, otherwise an error has ocurred (1 Byte)
    fread(&endmark, 1, 1, fp);
    if (endmark != 0xFF)
        return false;

    //we are finished, the surface is filled
    //fp should now point to the first offset of the next entry, this is the last check if this is REALLY the RIGHT position
    if ( ftell(fp) != (long int) next_entry )
    {
        fseek(fp, (long int) next_entry, SEEK_SET);
        return false;
    }


/**FOLLOWING COMMENTS ARE ABSOLUTLY TEMPORARY, UNTIL I KNOW HOW TO HANDLE SHADOWS WITH ALPHA-BLENDING AND THEN BLITTING
***---THIS CODE IS NOT USEFUL.**/
    //We have to blit picture and shadow together and because of transparency we need to set the colorkey
    //SDL_SetColorKey(shadowArray->surface, SDL_SRCCOLORKEY, SDL_MapRGBA(shadowArray->surface->format, 0, 0, 0, 0));
    //SDL_BlitSurface(shadowArray->surface, NULL, (shadowArray-1)->surface, NULL);
    //SDL_FreeSurface(shadowArray->surface);
    //increment shadowArray
    shadowArray++;

    free(starts);

    return true;
}


bool CFile::read_bob14(void)
{
    //length of data block
    Uint32 length;
    //offset of next entry in file
    long int next_entry;

    //Pic-Data
    //offset for the first byte of the data block
    long int data_start;
    //color value for read pixel
    Uint8 color_value;


    //skip unknown data (1x 2 Bytes)
    fseek(fp, 2, SEEK_CUR);
    //length of datablock (1x 4 Bytes)
    fread(&length, 4, 1, fp);
    //start offset of data block
    data_start = ftell(fp);
    //jump to first offset after data block
    fseek(fp, length, SEEK_CUR);
    //coordinate for zeropoint x (2 Bytes)
    fread(&(bmpArray->nx), 2, 1, fp);
    //coordinate for zeropoint y (2 Bytes)
    fread(&(bmpArray->ny), 2, 1, fp);
    //width of picture (2 Bytes)
    fread(&(bmpArray->w), 2, 1, fp);
    //heigth of picture (2 Bytes)
    fread(&(bmpArray->h), 2, 1, fp);
    //skip unknown data (8x 1 Byte)
    fseek(fp, 8, SEEK_CUR);

    //fp points now ON the first adress of the next entry in the file
    next_entry = ftell(fp);

    //if we only want to read a palette at the moment (loadPAL == 1) so skip this and let fp on the adress of the next entry
    if (loadPAL)
        return true;

    //now we are ready to read the picture lines and fill the surface, so lets create one
    if ( (bmpArray->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, bmpArray->w, bmpArray->h, 8, 0, 0, 0, 0)) == NULL )
        return false;
    SDL_SetPalette(bmpArray->surface, SDL_LOGPAL, palActual->colors, 0, 256);

    //set fp to back to the first offset of data block
    fseek(fp, data_start, SEEK_SET);

    //main loop for reading picture lines
    for (int y = 0; y < bmpArray->h; y++)
    {
        //loop for reading pixels of the actual picture line
        for (int x = 0; x < bmpArray->w; x++)
        {
            //read color value (1 Byte)
            fread(&color_value, 1, 1, fp);
            //draw
            CSurface::DrawPixel_Color(bmpArray->surface, x, y, (Uint32)color_value);
        }
    }

    //we are finished, the surface is filled
    //fp should now point to nx again, so set it to the next entry in the file
    fseek(fp, (long int) next_entry, SEEK_SET);

    //increment bmpArray for the next picture
    bmpArray++;

    return true;
}

inline void CFile::endian_swap(Uint16& x)
{
    x = (x>>8) |
    (x<<8);
}

inline void CFile::endian_swap(Uint32& x)
{
    x = (x>>24) |
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}