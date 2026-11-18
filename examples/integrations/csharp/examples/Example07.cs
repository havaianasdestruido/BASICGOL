using System;
using System.Runtime.InteropServices;

namespace Basicgol.Interop.CSharp;

internal static class Example07
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate int Transformar(int valor);

    [DllImport("basicgol", EntryPoint = "bg_aplica_callback", CallingConvention = CallingConvention.Cdecl)]
    private static extern int AplicarCallback(int valor, Transformar callback);

    private static int Triplicar(int valor) => valor * 3;

    public static void Run()
    {
        Transformar callback = Triplicar;
        Console.WriteLine($"Callback C# chamado por BASICGOL: {AplicarCallback(14, callback)}");
        GC.KeepAlive(callback);
    }
}
