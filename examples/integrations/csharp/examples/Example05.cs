using System;
using System.Runtime.InteropServices;

namespace Basicgol.Interop.CSharp;

internal static class Example05
{
    [DllImport("basicgol", EntryPoint = "bg_comprimento_utf8", CallingConvention = CallingConvention.Cdecl)]
    private static extern int ComprimentoUtf8(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string texto);

    public static void Run()
    {
        string texto = "ação";
        Console.WriteLine($"'{texto}' tem {ComprimentoUtf8(texto)} bytes UTF-8.");
    }
}
