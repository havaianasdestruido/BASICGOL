using System;
using System.Runtime.InteropServices;

namespace Basicgol.Interop.CSharp;

internal static class Example02
{
    [DllImport("basicgol", EntryPoint = "bg_fatorial", CallingConvention = CallingConvention.Cdecl)]
    private static extern long Fatorial(int n);

    public static void Run() => Console.WriteLine($"10! = {Fatorial(10)}");
}
