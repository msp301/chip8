#include <stdlib.h>
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

unsigned char font[ 80 ] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

void render( unsigned char gfx[] );

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


    // Load font sprites into memory
    for( int i = 0; i < 80; i++ )
    {
        memory[ i ] = font[ i ];
    }

    //printf( "NUM | CODE | ACTION" );

    for( pc = 0x200; pc < ( fsize + 0x200 ); pc += 2 )
    {
        unsigned char kk, n, x, y;
        unsigned short nnn, result;

        opcode = memory[ pc ] << 8 | memory[ pc + 1 ];

        //printf( "\n%03x | %04x | ", pc, opcode );

        switch( opcode & 0xFF00 )
        {
            case 0x0000:
                switch( opcode & 0x00FF )
                {
                    case 0x00E0:
                        //printf( "Clear screen" );

                        for( int i = 0; i < ( 64 * 32 ); i++ )
                        {
                            gfx[ i ] = 0;
                        }

                        render( gfx );

                        continue;
                    case 0x00EE:
                        //printf( "Return from subroutine" );

                        pc = stack[ stack_ptr ];
                        stack_ptr -= 1;

                        continue;
                }
                break;
        }

        switch( opcode & 0xF000 )
        {
            case 0x0000:
                continue;
            case 0x1000:
                nnn = ( opcode & 0x0FFF );

                //printf( "Jump to address: %04x", nnn );

                pc = nnn;

                continue;
            case 0x2000:
                nnn = ( opcode & 0x0FFF );

                //printf( "Call subroutine at address : %04x", nnn );

                stack_ptr++;
                stack[ stack_ptr ] = pc;
                pc = nnn;

                continue;
            case 0x3000:
                x  = ( opcode & 0x0F00 ) >> 8;
                kk = ( opcode & 0x00FF );

                //printf( "3xkk: Skip next if V%X = %02x", x, kk );

                if( V[ x ] == kk )
                {
                    pc += 2;
                }

                continue;
            case 0x4000:
                x  = ( opcode & 0x0F00 ) >> 8;
                kk = ( opcode & 0x00FF );

                //printf( "4xkk: Skip next if V%X != %02x", x, kk );

                if( V[ x ] != kk )
                {
                    pc += 2;
                }

                continue;
            case 0x5000:
                x = ( opcode & 0x0F00 ) >> 8;
                y = ( opcode & 0x00F0 ) >> 4;

                //printf( "5xy0: Skip next if V%X = V%X", x, y );

                if( V[ x ] == V[ y ] )
                {
                    pc += 2;
                }

                continue;
            case 0x6000:
                x  = ( opcode & 0x0F00 ) >> 8;
                kk = ( opcode & 0x00FF );

                //printf( "6xkk: Set V%d = %02x", x, kk );

                V[ x ] = kk;

                continue;
            case 0x7000:
                x  = ( opcode & 0x0F00 ) >> 8;
                kk = ( opcode & 0x00FF );

                //printf( "7xkk: Set V%X += %02x", x, kk );

                V[ x ] = V[ x ] + kk;

                continue;
            case 0x8000:
                x = ( opcode & 0x0F00 ) >> 8;
                y = ( opcode & 0x00F0 ) >> 4;

                switch( opcode & 0x000F )
                {
                    case 0x0000:
                        //printf( "8xy0: Set V%X = V%X", x, y );

                        V[ x ] = V[ y ];

                        continue;
                    case 0x0001:
                        //printf( "8xy1: Set V%X = V%X OR V%X", x, x, y );

                        V[ x ] = V[ x ] | V[ y ];

                        continue;
                    case 0x0002:
                        //printf( "8xy2: Set V%X = V%X AND V%X", x, x, y );

                        V[ x ] = V[ x ] & V[ y ];

                        continue;
                    case 0x0003:
                        //printf( "8xy3: Set V%X = V%X XOR V%X", x, x, y );

                        V[ x ] = V[ x ] ^ V[ y ];

                        continue;
                    case 0x0004:
                        //printf( "8xy4: Set V%X += V%X, Set VF = carry", x, y );

                        result = V[ x ] + V[ y ];
                        V[ x ] = result & 0x00FF;
                        V[ 0xF ] = ( result > 0x00FF ) ? 1 : 0;

                        continue;
                    case 0x0005:
                        //printf( "8xy5: Set V%X -= V%X, Set VF = NOT borrow", x, y );

                        V[ 0xF ] = ( V[ x ] > V[ y ] ) ? 1 : 0;
                        V[ x ] -= V[ y ];

                        continue;
                    case 0x0006:
                        //printf( "8xy6: Set V%X = Vx SHR 1", x );

                        V[ 0xF ] = V[ x ] & 1;
                        V[ x ] /= 2;

                        continue;
                    case 0x0007:
                        //printf( "8xy7: Set V%X = V%X - V%X, Set VF = NOT borrow", x, y, x );

                        V[ 0xF ] = ( V[ y ] > V[ x ] ) ? 1 : 0;
                        V[ x ] = V[ y ] - V[ x ];

                        continue;
                    case 0x000E:
                        //printf( "8xyE: Set V%X = V%X SHL 1", x, x );

                        V[ 0xF ] = ( V[ x ] & 128 ) >> 7;
                        V[ x ] *= 2;

                        continue;
                }
                break;
            case 0x9000:
                x = ( opcode & 0x0F00 ) >> 8;
                y = ( opcode & 0x00F0 ) >> 4;

                //printf( "9xy0: Skip next if V%X != V%X", x, y );

                if( V[ x ] != V[ y ] )
                {
                    pc += 2;
                }

                continue;
            case 0xA000:
                nnn = ( opcode & 0x0FFF );

                //printf( "Annn: Set I = %03x", opcode & 0x0FFF );

                I = nnn;

                continue;
            case 0xB000:
                nnn = ( opcode & 0x0FFF );

                //printf( "Bnnn: Jump to address %03x + V0", opcode & 0x0FFF );

                pc = nnn + V[ 0 ];

                continue;
            case 0xC000:
                x  = ( opcode & 0x0F00 ) >> 8;
                kk = ( opcode & 0x00FF );

                //printf( "Cxkk: V%X = rand byte AND %02x", x, kk );

                V[ x ] = ( rand() % 255 ) & kk;

                continue;
            case 0xD000:
                x = ( opcode & 0x0F00 ) >> 8;
                y = ( opcode & 0x00F0 ) >> 4;
                n = ( opcode & 0x000F );

                //printf( "Dxyn: Display sprite" );

                V[ 0xF ] = 0;
                for( int y_line = I; y_line < ( I + n ); y_line++ )
                {
                    for( int x_line = 0; x_line < 8; x_line++ )
                    {
                        if( gfx[ x + x_line + ( y + y_line * 64 ) ] == 1 )
                        {
                            V[ 0xF ] = 1;
                        }

                        gfx[ x + x_line + ( y + y_line * 64 ) ] ^= 1;
                    }
                }


                for( int i = 0; i < ( 64 * 32 ); i++ )
                {
                    gfx[ i ] = 0;
                }

                render( gfx );

                continue;
            case 0xE000:
                switch( opcode & 0x00FF )
                {
                    case 0x009E:
                        //printf( "Ex9E: Skip next if keypress = Vx" );
                        continue;
                    case 0x00A1:
                        //printf( "ExA1: Skip next if keypress != Vx" );
                        continue;
                }
                break;
            case 0xF000:
                switch( opcode & 0x00FF )
                {
                    case 0x0007:
                        //printf( "Fx07: Vx = delay timer" );
                        continue;
                    case 0x000A:
                        //printf( "Fx0A: Wait for key press. Store in Vx" );
                        continue;
                    case 0x0018:
                        //printf( "Fx18: Sound timer = Vx" );
                        continue;
                    case 0x001E:
                        //printf( "Fx1E: I = I + Vx" );
                        continue;
                    case 0x0029:
                        x  = ( opcode & 0x0F00 ) >> 8;

                        //printf( "Fx29: I = Sprite location for digit Vx" );

                        I = V[ x ] * 5;

                        continue;
                    case 0x0033:
                        //printf( "Fx33: BCD of Vx at I, I+1 and I+2" );
                        continue;
                    case 0x0055:
                        //printf( "Fx55: Store registers V0 - Vx in memory starting at location I" );
                        continue;
                    case 0x0065:
                        //printf( "Fx65: Read registers V0 - Vx from memory starting at location I" );
                        continue;
                }
        }
    }

    return 0;
}

void render( unsigned char gfx[] )
{
    for( int i = 0; i < ( 64 * 32 ); i++ )
    {
        int pixel = gfx[ i ];
        printf( "%d", pixel );

        if( ( ( i + 1 ) % 64 ) == 0 )
        {
            printf("\n");
        }
    }

    printf("\n\n\n");

    return;
}
