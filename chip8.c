#include <stdio.h>

// TODO:
// 1. Startup
// 2. Initialise CPU
// 3. Load ROM
// 4. Start loop
// 5. Emulate CPU cycle
// 6. Update screen if draw flag set
// 7. Read keypad
// 8. Goto step 5

int main(int argc, char** argv)
{
    unsigned char memory[ 4096 ]; // 4K 8-bit memory
    unsigned char V[ 16 ];        // 16 8-bit registers (V0-VF)
    unsigned short I;             // 16-bit Index register
    unsigned short pc;            // Program counter: 0x000 - 0xFFF
    unsigned short opcode;

    unsigned char gfx[ 64 * 32 ]; // 64x32 pixel display

    unsigned char delay_timer;
    unsigned char sound_timer;

    unsigned short stack[ 16 ];
    unsigned short stack_ptr;

    unsigned char keypad[ 16 ];

    // Load supplied ROM
    FILE *file = fopen( argv[1], "rb" );
    if( file == NULL )
    {
        printf( "Failed to open ROM" );
        return 1;
    }

    // Determine the ROM file size
    fseek( file, 0L, SEEK_END );
    int fsize = ftell( file );
    fseek( file, 0L, SEEK_SET );

    // Store ROM in memory
    fread( memory+0x200, fsize, 1, file );
    fclose( file );

    printf( "NUM | CODE | ACTION" );

    pc = 0x200;
    while( pc < ( fsize + 0x200 ) )
    {
        // Read the opcode
        opcode = memory[ pc ] << 8 | memory[ pc + 1 ];

        printf( "\n%03hhX | %04hhX | ", pc, opcode );

        pc += 2;
        int unknown = 0;
        switch( opcode & 0x00FF )
        {
            case 0x00E0:
                printf( "Clear screen" );
                break;
            case 0x00EE:
                printf( "Return from subroutine" );
                break;
            default:
                unknown = 1;
        }

        if( unknown == 0 )
        {
            continue;
        }

        switch( opcode & 0xF000 )
        {
            case 0x0000:
                printf( "Execute subroutine at address: %04hhX", opcode );
                break;
        }
    }

    return 0;
}
