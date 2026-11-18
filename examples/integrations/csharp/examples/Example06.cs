using System;
using System.Runtime.InteropServices;

namespace Basicgol.Interop.CSharp;

internal static class Example06
{
    [DllImport("basicgol", EntryPoint = "bg_mensagem_utf8", CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr MensagemUtf8();

    public static void Run()
    {
        string? texto = Marshal.PtrToStringUTF8(MensagemUtf8());
        Console.WriteLine(texto);
    }
}
