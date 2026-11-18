using System;
using System.Collections.Generic;

namespace Basicgol.Interop.CSharp;

internal static class Program
{
    private static readonly IReadOnlyDictionary<string, Action> Examples =
        new Dictionary<string, Action>
        {
            ["01"] = Example01.Run,
            ["02"] = Example02.Run,
            ["03"] = Example03.Run,
            ["04"] = Example04.Run,
            ["05"] = Example05.Run,
            ["06"] = Example06.Run,
            ["07"] = Example07.Run,
        };

    private static int Main(string[] args)
    {
        if (args.Length == 0 || args[0] == "--list")
        {
            Console.WriteLine("Escolha um exemplo: 01, 02, 03, 04, 05, 06 ou 07.");
            return 0;
        }

        if (!Examples.TryGetValue(args[0], out var run))
        {
            Console.Error.WriteLine($"Exemplo desconhecido: {args[0]}");
            return 2;
        }

        run();
        return 0;
    }
}
