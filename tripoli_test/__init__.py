try:
    # Check that we are running embedded
    import emacs_raw

except ImportError:
    print("Can't run tests outside Emacs embedding.")

else:
    try:
        from os.path import dirname
        import pytest
        import sys

        # Fool pytest into running
        sys.argv = ['tripoli']

        module = dirname(__file__)
        pytest.main(['-v', module])

    except Exception as e:
        print('{}: {}'.format(e.__class__.__name__, e))
        _, _, tb = sys.exc_info()
        traceback.print_tb(tb)
