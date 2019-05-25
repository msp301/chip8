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

    for( pc = 0x200; pc < ( fsize + 0x200 ); pc += 2 )
    {
        opcode = memory[ pc ] << 8 | memory[ pc + 1 ];

        printf( "\n%03x | %04x | ", pc, opcode );

        switch( opcode & 0xFF00 )
        {
            case 0x0000:
                switch( opcode & 0x00FF )
                {
                    case 0x00E0:
                        printf( "Clear screen" );
                        continue;
                    case 0x00EE:
                        printf( "Return from subroutine" );
                        continue;
                }
                break;
        }

        switch( opcode & 0xF000 )
        {
            case 0x0000:
                printf( "Execute subroutine at address: %04x", opcode & 0x0FFF );
                continue;
            case 0x1000:
                printf( "Jump to address: %04x", opcode & 0x0FFF );
                continue;
            case 0x2000:
                printf( "Call subroutine at address : %04x", opcode & 0x0FFF );
                continue;
            case 0x3000:
                printf( "3xkk: Skip next if Vx = kk" );
                continue;
            case 0x4000:
                printf( "4xkk: Skip next if Vx != kk" );
                continue;
            case 0x5000:
                printf( "5xy0: Skip next if Vx = Vy" );
                continue;
            case 0x6000:
                printf( "6xkk: Set Vx = kk" );
                continue;
            case 0x7000:
                printf( "7xkk: Set Vx = Vx + kk" );
                continue;
        }
    }

    return 0;
}
